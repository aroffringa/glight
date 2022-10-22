#include "management.h"

#include "beatfinder.h"
#include "chase.h"
#include "controllable.h"
#include "dmxdevice.h"
#include "dummydevice.h"
#include "effect.h"
#include "fixturecontrol.h"
#include "folder.h"
#include "presetcollection.h"
#include "presetvalue.h"
#include "sequence.h"
#include "show.h"
#include "sourcevalue.h"
#include "theatre.h"
#include "timesequence.h"
#include "valuesnapshot.h"

namespace glight::theatre {

Management::Management()
    : _thread(),
      _isQuitting(false),
      _createTime(std::chrono::steady_clock::now()),
      _rndDistribution(0, ControlValue::MaxUInt() + 1),
      _overridenBeat(0),
      _lastOverridenBeatTime(0.0),

      _theatre(new class Theatre()),
      _primarySnapshot(new ValueSnapshot(true, 0)),
      _secondarySnapshot(new ValueSnapshot(false, 0)),
      _show(new class Show(*this)) {
  _folders.emplace_back(new Folder());
  _rootFolder = _folders.back().get();
  _rootFolder->SetName("Root");
}

Management::~Management() {
  if (_thread) {
    _isQuitting = true;
    abortAllDevices();
    _thread->join();
    _thread.reset();
  }
}

void Management::StartBeatFinder() {
  _beatFinder.reset(new BeatFinder());
  _beatFinder->Start();
}

void Management::Clear() {
  _show->Clear();

  _controllables.clear();
  _sourceValues.clear();
  _folders.clear();
  _folders.emplace_back(new Folder());
  _rootFolder = _folders.back().get();
  _rootFolder->SetName("Root");

  _theatre->Clear();
}

void Management::AddDevice(std::unique_ptr<class DmxDevice> device) {
  std::lock_guard<std::mutex> lock(_mutex);
  _devices.emplace_back(std::move(device));
  _primarySnapshot->SetUniverseCount(_devices.size());
  _secondarySnapshot->SetUniverseCount(_devices.size());
}

void Management::Run() {
  if (_thread == nullptr) {
    _isQuitting = false;
    _thread.reset(new std::thread([&]() { ThreadLoop(); }));
  } else
    throw std::runtime_error("Invalid call to Run(): already running");
}

void Management::ThreadLoop() {
  std::unique_ptr<ValueSnapshot> nextPrimary =
      std::make_unique<ValueSnapshot>(true, _devices.size());
  std::unique_ptr<ValueSnapshot> nextSecondary =
      std::make_unique<ValueSnapshot>(false, _devices.size());
  unsigned timestepNumber = 0;
  while (!_isQuitting) {
    for (bool is_primary : {true, false}) {
      for (unsigned universe = 0; universe != _devices.size(); ++universe) {
        unsigned values[512];
        unsigned char valuesChar[512];

        std::fill_n(values, 512, 0);

        getChannelValues(timestepNumber, values, universe, is_primary);

        for (unsigned i = 0; i < 512; ++i) {
          unsigned val = (values[i] >> 16);
          if (val > 255) val = 255;
          valuesChar[i] = static_cast<unsigned char>(val);
        }

        ValueUniverseSnapshot &universeValues =
            is_primary ? nextPrimary->GetUniverseSnapshot(universe)
                       : nextSecondary->GetUniverseSnapshot(universe);
        universeValues.SetValues(valuesChar, _theatre->HighestChannel() + 1);

        if (is_primary) {
          _devices[universe]->WaitForNextSync();
          _devices[universe]->SetValues(valuesChar, 512);
        }
      }
    }

    std::lock_guard<std::mutex> lock(_mutex);
    std::swap(_primarySnapshot, nextPrimary);
    std::swap(_secondarySnapshot, nextSecondary);

    ++timestepNumber;
  }
}

void Management::abortAllDevices() {
  for (std::unique_ptr<DmxDevice> &device : _devices) {
    device->Abort();
  }
}

void Management::getChannelValues(unsigned timestepNumber, unsigned *values,
                                  unsigned universe, bool primary) {
  double relTimeInMs = GetOffsetTimeInMS();
  double beatValue;
  unsigned audioLevel;
  // Get the beat
  if (relTimeInMs - _lastOverridenBeatTime < 8000.0 && _overridenBeat != 0) {
    beatValue = _overridenBeat;
  } else if (_beatFinder) {
    double beatConfidence;
    _beatFinder->GetBeatValue(beatValue, beatConfidence);
  } else {
    beatValue = 0.0;
  }

  if (_beatFinder)
    audioLevel = _beatFinder->GetAudioLevel();
  else
    audioLevel = 0;

  const unsigned randomValue = _rndDistribution(_randomGenerator);
  Timing timing(relTimeInMs, timestepNumber, beatValue, audioLevel,
                randomValue);
  const double timePassed = (relTimeInMs - _previousTime) * 1e-3;
  _previousTime = relTimeInMs;
  for (std::unique_ptr<SourceValue> &sv : _sourceValues) {
    sv->ApplyFade(timePassed);
  }

  std::lock_guard<std::mutex> lock(_mutex);

  _show->Mix(values, universe, timing);

  // Reset all inputs
  for (const std::unique_ptr<SourceValue> &sv : _sourceValues) {
    for (size_t inputIndex = 0; inputIndex != sv->GetControllable().NInputs();
         ++inputIndex) {
      sv->GetControllable().InputValue(inputIndex) = 0;
    }
  }

  if(primary) {
    for (const std::unique_ptr<SourceValue> &sv : _sourceValues)
      sv->GetControllable().MixInput(sv->InputIndex(), sv->A().Value());
  }
  else {
    for (const std::unique_ptr<SourceValue> &sv : _sourceValues)
      sv->GetControllable().MixInput(sv->InputIndex(), sv->B().Value());
  }

  // Solve dependency graph of controllables
  std::vector<Controllable *> unorderedList;
  for (const std::unique_ptr<Controllable> &c : _controllables)
    unorderedList.emplace_back(c.get());
  std::vector<Controllable *> orderedList;
  if (!topologicalSort(unorderedList, orderedList))
    throw std::runtime_error("Cycle in dependencies");

  for (auto c = orderedList.rbegin(); c != orderedList.rend(); ++c) {
    Controllable *controllable = *c;
    controllable->Mix(timing);
    if (FixtureControl *fc = dynamic_cast<FixtureControl *>(controllable)) {
      fc->MixChannels(values, universe);
    }
  }
}

bool Management::HasCycle() const {
  std::vector<Controllable *> unorderedList, orderedList;
  for (const std::unique_ptr<Controllable> &c : _controllables)
    unorderedList.emplace_back(c.get());
  return !topologicalSort(unorderedList, orderedList);
}

PresetCollection &Management::AddPresetCollection() {
  return static_cast<PresetCollection &>(
      *_controllables.emplace_back(std::make_unique<PresetCollection>()));
}

Folder &Management::AddFolder(Folder &parent, const std::string &name) {
  _folders.emplace_back(new Folder(name));
  parent.Add(*_folders.back());
  return *_folders.back();
}

Folder &Management::GetFolder(const std::string &path) {
  return *_rootFolder->FollowDown(Folder::RemoveRoot(path));
}

void Management::RemoveObject(FolderObject &object) {
  Folder *folder = dynamic_cast<Folder *>(&object);
  if (folder)
    RemoveFolder(*folder);
  else {
    Controllable *controllable = dynamic_cast<Controllable *>(&object);
    if (controllable)
      RemoveControllable(*controllable);
    else
      throw std::runtime_error("Can not remove unknown object " +
                               object.Name());
  }
}

void Management::RemoveFolder(Folder &folder) {
  if (&folder == _rootFolder)
    throw std::runtime_error("Can not remove root folder");
  // Removing a child might remove dependent children from the same folder
  // so we have to recheck whether the folder is empty after each removal
  while (!folder.Children().empty()) {
    RemoveObject(*folder.Children().back());
  }
  folder.Parent().Remove(folder);
  for (auto iter = _folders.begin(); iter != _folders.end(); ++iter) {
    if (iter->get() == &folder) {
      _folders.erase(iter);
      break;
    }
  }
}

void Management::RemoveControllable(Controllable &controllable) {
  removeControllable(_controllables.begin() +
                     FolderObject::FindIndex(_controllables, &controllable));
}

void Management::removeControllable(
    std::vector<std::unique_ptr<Controllable>>::iterator controllablePtr) {
  std::unique_ptr<Controllable> controllable = std::move(*controllablePtr);

  _controllables.erase(controllablePtr);

  auto result =
      std::remove_if(_sourceValues.begin(), _sourceValues.end(),
                     [&controllable](std::unique_ptr<SourceValue> &pv) {
                       return &pv->GetControllable() == controllable.get();
                     });
  _sourceValues.erase(result, _sourceValues.end());

  controllable->Parent().Remove(*controllable.get());

  std::vector<std::unique_ptr<Controllable>>::iterator i =
      _controllables.begin();
  while (i != _controllables.end()) {
    if ((*i)->HasOutputConnection(*controllable)) {
      removeControllable(i);
      // Every time we remove something, we have to restart, because the vector
      // might have changed because of other dependencies
      i = _controllables.begin();
    } else {
      ++i;
    }
  }
}

bool Management::Contains(Controllable &controllable) const {
  for (const std::unique_ptr<Controllable> &contr : _controllables) {
    if (contr.get() == &controllable) return true;
  }
  return false;
}

FixtureControl &Management::AddFixtureControl(const Fixture &fixture) {
  _controllables.emplace_back(
      new FixtureControl(const_cast<Fixture &>(fixture)));
  return static_cast<FixtureControl &>(*_controllables.back());
}

FixtureControl &Management::AddFixtureControl(const Fixture &fixture,
                                              const Folder &parent) {
  _controllables.emplace_back(
      new FixtureControl(const_cast<Fixture &>(fixture)));
  const_cast<Folder &>(parent).Add(*_controllables.back());
  return static_cast<FixtureControl &>(*_controllables.back());
}

FixtureControl &Management::GetFixtureControl(const Fixture &fixture) {
  for (const std::unique_ptr<Controllable> &contr : _controllables) {
    FixtureControl *fc = dynamic_cast<FixtureControl *>(contr.get());
    if (fc) {
      if (&fc->Fixture() == &fixture) return *fc;
    }
  }
  throw std::runtime_error("GetFixtureControl() : Fixture control not found");
}

void Management::RemoveFixture(const Fixture &fixture) {
  FixtureControl &control = GetFixtureControl(fixture);
  _theatre->RemoveFixture(fixture);
  RemoveControllable(control);
}

void Management::RemoveFixtureType(const FixtureType &fixtureType) {
  const std::vector<std::unique_ptr<Fixture>> &fixtures = _theatre->Fixtures();
  bool isUsed = false;
  size_t i = 0;
  while (i != fixtures.size()) {
    // Go backward through the list, as fixtures might be removed
    const size_t fIndex = fixtures.size() - 1 - i;
    Fixture &f = *fixtures[fIndex];
    if (&f.Type() == &fixtureType) {
      RemoveFixture(*fixtures[fIndex]);
      isUsed = true;
    } else {
      ++i;
    }
  }
  // When the fixture type was used, removing the last fixture of that type
  // will remove the type (elsewhere). Otherwise, remove it manually.
  if (!isUsed) {
    _theatre->RemoveFixtureType(fixtureType);
  }
}

SourceValue &Management::AddSourceValue(Controllable &controllable,
                                        size_t inputIndex) {
  _sourceValues.emplace_back(
      std::make_unique<SourceValue>(controllable, inputIndex));
  return *_sourceValues.back();
}

void Management::RemoveSourceValue(SourceValue &sourceValue) {
  for (std::vector<std::unique_ptr<SourceValue>>::iterator i =
           _sourceValues.begin();
       i != _sourceValues.end(); ++i) {
    if (i->get() == &sourceValue) {
      _sourceValues.erase(i);
      return;
    }
  }
}

bool Management::Contains(SourceValue &sourceValue) const {
  for (const std::unique_ptr<SourceValue> &sv : _sourceValues) {
    if (sv.get() == &sourceValue) return true;
  }
  return false;
}

Chase &Management::AddChase() {
  _controllables.emplace_back(new Chase());
  return static_cast<Chase &>(*_controllables.back());
}

TimeSequence &Management::AddTimeSequence() {
  _controllables.emplace_back(new TimeSequence());
  return static_cast<TimeSequence &>(*_controllables.back());
}

Effect &Management::AddEffect(std::unique_ptr<Effect> effect) {
  _controllables.emplace_back(std::move(effect));
  return static_cast<Effect &>(*_controllables.back());
}

Effect &Management::AddEffect(std::unique_ptr<Effect> effect, Folder &folder) {
  Effect &newEffect = AddEffect(std::move(effect));
  folder.Add(newEffect);
  return newEffect;
}

FolderObject *Management::GetObjectFromPathIfExists(
    const std::string &path) const {
  auto sep = std::find(path.begin(), path.end(), '/');
  if (sep == path.end()) {
    if (path == _rootFolder->Name()) return _rootFolder;
  } else {
    std::string left = path.substr(0, sep - path.begin());
    std::string right = path.substr(sep + 1 - path.begin());
    if (left == _rootFolder->Name()) return _rootFolder->FollowRelPath(right);
  }
  return nullptr;
}

FolderObject &Management::GetObjectFromPath(const std::string &path) {
  FolderObject *result = GetObjectFromPathIfExists(path);
  if (result)
    return *result;
  else
    throw std::runtime_error("Could not find object with path " + path);
}

size_t Management::ControllableIndex(const Controllable *controllable) const {
  return FolderObject::FindIndex(_controllables, controllable);
}

SourceValue *Management::GetSourceValue(const Controllable &controllable,
                                        size_t inputIndex) {
  for (const std::unique_ptr<SourceValue> &sv : _sourceValues)
    if (&sv->GetControllable() == &controllable &&
        sv->InputIndex() == inputIndex)
      return sv.get();
  return nullptr;
}

size_t Management::SourceValueIndex(const SourceValue *sourceValue) const {
  return NamedObject::FindIndex(_sourceValues, sourceValue);
}

ValueSnapshot Management::PrimarySnapshot() {
  std::lock_guard<std::mutex> lock(_mutex);
  return *_primarySnapshot;
}

ValueSnapshot Management::SecondarySnapshot() {
  std::lock_guard<std::mutex> lock(_mutex);
  return *_secondarySnapshot;
}

ValueSnapshot Management::Snapshot(bool primary) {
  if (primary)
    return PrimarySnapshot();
  else
    return SecondarySnapshot();
}

bool Management::topologicalSort(const std::vector<Controllable *> &input,
                                 std::vector<Controllable *> &output) {
  for (Controllable *controllable : input) controllable->SetVisitLevel(0);
  for (Controllable *controllable : input) {
    if (!topologicalSortVisit(*controllable, output)) return false;
  }
  return true;
}

bool Management::topologicalSortVisit(Controllable &controllable,
                                      std::vector<Controllable *> &list) {
  if (controllable.VisitLevel() == 0) {
    controllable.SetVisitLevel(1);
    for (size_t i = 0; i != controllable.NOutputs(); ++i) {
      Controllable *other = controllable.Output(i).first;
      if (!topologicalSortVisit(*other, list)) return false;
    }
    controllable.SetVisitLevel(2);
    list.emplace_back(&controllable);
  } else if (controllable.VisitLevel() == 1)
    return false;
  return true;
}

void Management::BlackOut() {
  for (std::unique_ptr<SourceValue> &sv : _sourceValues) {
    sv->A().Set(0, 0.0);
  }
}

}  // namespace glight::theatre
