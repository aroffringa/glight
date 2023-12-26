#include "management.h"

#include <cmath>

#include <ranges>

#include "beatfinder.h"
#include "chase.h"
#include "controllable.h"
#include "dmxdevice.h"
#include "dummydevice.h"
#include "effect.h"
#include "fixturecontrol.h"
#include "fixturegroup.h"
#include "folder.h"
#include "folderoperations.h"
#include "presetcollection.h"
#include "presetvalue.h"
#include "sequence.h"
#include "sourcevalue.h"
#include "theatre.h"
#include "timesequence.h"
#include "valuesnapshot.h"

#include "scenes/scene.h"

namespace glight::theatre {

Management::Management()
    : _isQuitting(false),
      _createTime(std::chrono::steady_clock::now()),
      _rndDistribution(0, ControlValue::MaxUInt() + 1),
      _overridenBeat(0),
      _lastOverridenBeatTime(0.0),

      _theatre(std::make_unique<Theatre>()),
      _primarySnapshot(std::make_unique<ValueSnapshot>(true, 0)),
      _secondarySnapshot(std::make_unique<ValueSnapshot>(false, 0)) {
  _rootFolder = _folders.emplace_back(std::make_unique<Folder>()).get();
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
  _beatFinder = std::make_unique<BeatFinder>();
  _beatFinder->Start();
}

void Management::Clear() {
  _controllables.clear();
  _groups.clear();
  _sourceValues.clear();
  _folders.clear();
  _rootFolder = _folders.emplace_back(std::make_unique<Folder>()).get();
  _rootFolder->SetName("Root");

  _theatre->Clear();
}

void Management::AddDevice(std::unique_ptr<DmxDevice> device) {
  std::lock_guard<std::mutex> lock(_mutex);
  _device = std::move(device);
  const size_t n_universes = _device->NUniverses();
  _primarySnapshot->SetUniverseCount(n_universes);
  _secondarySnapshot->SetUniverseCount(n_universes);
}

void Management::Run() {
  if (_thread == nullptr) {
    _isQuitting = false;
    _thread = std::make_unique<std::thread>([&]() { ThreadLoop(); });
  } else
    throw std::runtime_error("Invalid call to Run(): already running");
}

void Management::ProcessInputUniverse(unsigned universe,
                                      ValueSnapshot &snapshot,
                                      bool is_primary) {
  unsigned values[512];
  unsigned char values_char[512];

  std::fill_n(values, 512, 0);

  for (const std::unique_ptr<Controllable> &controllable : _controllables) {
    if (FixtureControl *fc =
            dynamic_cast<FixtureControl *>(controllable.get())) {
      fc->GetChannelValues(values, universe);
    }
  }

  for (unsigned i = 0; i < 512; ++i) {
    unsigned val = (values[i] >> 16);
    if (val > 255) val = 255;
    values_char[i] = static_cast<unsigned char>(val);
  }

  ValueUniverseSnapshot &universe_values =
      snapshot.GetUniverseSnapshot(universe);
  universe_values.SetValues(values_char, _theatre->HighestChannel() + 1);

  if (is_primary) {
    _device->SetOutputValues(universe, values_char, 512);
  }
}

void Management::ThreadLoop() {
  const size_t n_universes = _device->NUniverses();
  std::unique_ptr<ValueSnapshot> next_primary =
      std::make_unique<ValueSnapshot>(true, n_universes);
  std::unique_ptr<ValueSnapshot> next_secondary =
      std::make_unique<ValueSnapshot>(false, n_universes);
  unsigned timestep_number = 0;
  while (!_isQuitting) {
    MixAll(timestep_number, *next_primary, *next_secondary);
    _device->WaitForNextSync();

    std::lock_guard<std::mutex> lock(_mutex);
    std::swap(_primarySnapshot, next_primary);
    std::swap(_secondarySnapshot, next_secondary);

    ++timestep_number;
  }
}

void Management::abortAllDevices() { _device->Abort(); }

void Management::MixAll(unsigned timestep_number, ValueSnapshot &primary,
                        ValueSnapshot &secondary) {
  const double relTimeInMs = GetOffsetTimeInMS();
  double beatValue = 0.0;
  unsigned audioLevel = 0;
  // Get the beat
  if (relTimeInMs - _lastOverridenBeatTime < 8000.0 && _overridenBeat != 0) {
    beatValue = _overridenBeat;
  } else if (_beatFinder) {
    double beatConfidence = 0.0;
    _beatFinder->GetBeatValue(beatValue, beatConfidence);
  } else {
    beatValue = 0.0;
  }

  if (_beatFinder)
    audioLevel = _beatFinder->GetAudioLevel();
  else
    audioLevel = 0;

  const unsigned randomValue = _rndDistribution(_randomGenerator);
  const Timing timing(relTimeInMs, timestep_number, beatValue, audioLevel,
                      randomValue);
  const double timePassed = (relTimeInMs - _previousTime) * 1e-3;
  _previousTime = relTimeInMs;

  std::lock_guard<std::mutex> lock(_mutex);
  for (std::unique_ptr<SourceValue> &sv : _sourceValues) {
    sv->ApplyFade(timePassed);
  }

  // Solve dependency graph of controllables
  std::vector<Controllable *> unorderedList;
  for (const std::unique_ptr<Controllable> &c : _controllables)
    unorderedList.emplace_back(c.get());
  std::vector<Controllable *> orderedList;
  if (!topologicalSort(unorderedList, orderedList))
    throw std::runtime_error("Cycle in dependencies");

  for (bool is_primary : {false, true}) {
    // Reset all inputs
    for (const std::unique_ptr<SourceValue> &sv : _sourceValues) {
      for (size_t inputIndex = 0; inputIndex != sv->GetControllable().NInputs();
           ++inputIndex) {
        sv->GetControllable().InputValue(inputIndex) = ControlValue(0);
      }
    }

    // Process source values. These will output to controllables.
    if (is_primary) {
      for (const std::unique_ptr<SourceValue> &sv : _sourceValues)
        sv->GetControllable().MixInput(sv->InputIndex(),
                                       ControlValue(sv->PrimaryValue()));
    } else {
      for (const std::unique_ptr<SourceValue> &sv : _sourceValues)
        sv->GetControllable().MixInput(sv->InputIndex(),
                                       ControlValue(sv->SecondaryValue()));
    }

    // Process all controllables that follow
    for (Controllable *controllable : std::ranges::reverse_view(orderedList)) {
      controllable->Mix(timing, is_primary);
    }

    // All controllables have provided their output; now obtain the DMX values
    // and store them in the ValueSnapshot.
    const unsigned n_universes = _device->NUniverses();
    ValueSnapshot &snapshot = is_primary ? primary : secondary;
    for (unsigned universe = 0; universe != n_universes; ++universe) {
      if (_device->GetUniverseType(universe) == UniverseType::Output) {
        ProcessInputUniverse(universe, snapshot, is_primary);
      }
    }
  }
}

bool Management::HasCycle() const {
  std::vector<Controllable *> unorderedList;
  std::vector<Controllable *> orderedList;
  for (const std::unique_ptr<Controllable> &c : _controllables)
    unorderedList.emplace_back(c.get());
  return !topologicalSort(unorderedList, orderedList);
}

PresetCollection &Management::AddPresetCollection() {
  return static_cast<PresetCollection &>(
      *_controllables.emplace_back(std::make_unique<PresetCollection>()));
}

Folder &Management::AddFolder(Folder &parent, const std::string &name) {
  _folders.emplace_back(std::make_unique<Folder>(name));
  parent.Add(*_folders.back());
  return *_folders.back();
}

Folder &Management::GetFolder(const std::string &path) {
  return *_rootFolder->FollowDown(folders::RemoveRoot(path));
}

void Management::RemoveObject(FolderObject &object) {
  if (Folder *folder = dynamic_cast<Folder *>(&object); folder)
    RemoveFolder(*folder);
  else if (FixtureGroup *group = dynamic_cast<FixtureGroup *>(&object); group)
    RemoveFixtureGroup(*group);
  else if (Controllable *controllable = dynamic_cast<Controllable *>(&object);
           controllable)
    RemoveControllable(*controllable);
  else
    throw std::runtime_error("Can not remove unknown object " + object.Name());
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

  controllable->Parent().Remove(*controllable);

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
      if (&fc->GetFixture() == &fixture) return *fc;
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

FixtureGroup &Management::AddFixtureGroup() {
  return *_groups.emplace_back(std::make_unique<FixtureGroup>());
}

FixtureGroup &Management::AddFixtureGroup(const Folder &parent,
                                          const std::string &name) {
  FixtureGroup &group =
      *_groups.emplace_back(std::make_unique<FixtureGroup>(name));
  const_cast<Folder &>(parent).Add(group);
  return group;
}

void Management::RemoveFixtureGroup(const FixtureGroup &group) {
  for (std::vector<std::unique_ptr<FixtureGroup>>::iterator i = _groups.begin();
       i != _groups.end(); ++i) {
    if (i->get() == &group) {
      i->get()->Parent().Remove(**i);
      _groups.erase(i);
      return;
    }
  }
  assert(false);
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
  assert(false);
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

Scene &Management::AddScene(bool in_folder) {
  std::unique_ptr<Controllable> &result =
      _controllables.emplace_back(std::make_unique<Scene>(*this));
  if (in_folder) {
    result->SetName(_rootFolder->GetAvailableName("scene"));
    _rootFolder->Add(*result);
  }
  return static_cast<Scene &>(*result);
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

FolderObject &Management::GetObjectFromPath(const std::string &path) const {
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

void Management::BlackOut(bool skip_scenes, double fade_speed) {
  for (std::unique_ptr<SourceValue> &source_value : _sourceValues) {
    Controllable &controllable = source_value->GetControllable();
    if (!skip_scenes || !dynamic_cast<Scene *>(&controllable)) {
      source_value->A().Set(0, fade_speed);
      source_value->B().Set(0, fade_speed);
    }
  }
}

SourceValueStore Management::StoreSourceValues(bool use_a) const {
  SourceValueStore result;
  for (const std::unique_ptr<glight::theatre::SourceValue> &source_value :
       _sourceValues) {
    const ControlValue value =
        use_a ? source_value->A().Value() : source_value->B().Value();
    if (value) {
      Controllable &controllable = source_value->GetControllable();
      if (!dynamic_cast<Scene *>(&controllable)) {
        result.AddItem(*source_value, value);
      }
    }
  }
  return result;
}

void Management::LoadSourceValues(const SourceValueStore &store, bool use_a,
                                  double fade_speed) {
  const std::vector<SourceValueStoreItem> &items = store.GetItems();
  for (const SourceValueStoreItem &item : items) {
    SourceValue &source_value = item.GetSourceValue();
    if (use_a)
      source_value.A().Set(item.GetValue().UInt(), fade_speed);
    else
      source_value.B().Set(item.GetValue().UInt(), fade_speed);
  }
}

}  // namespace glight::theatre
