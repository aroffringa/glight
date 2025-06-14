#include "management.h"

#include <cmath>

#include <ranges>

#include "chase.h"
#include "controllable.h"
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

#include "devices/beatfinder.h"

#include "system/settings.h"

#include "scenes/scene.h"

namespace glight::theatre {

using system::MakeTrackable;
using system::ObservingPtr;
using system::TrackablePtr;

Management::Management(const system::Settings &settings)
    : settings_(settings), _theatre(std::make_unique<Theatre>()) {
  _rootFolder = _folders.emplace_back(MakeTrackable<Folder>()).Get();
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
  std::lock_guard<std::mutex> lock(_mutex);
  // In case the beat finder is already running, it is better to stop it first
  // so the audio device is not used twice.
  _beatFinder.reset();
  _beatFinder = std::make_unique<BeatFinder>(settings_.audio_input);
  _beatFinder->Start();
}

void Management::Clear() {
  _controllables.clear();
  _groups.clear();
  _sourceValues.clear();
  _folders.clear();
  _rootFolder = _folders.emplace_back(std::make_unique<Folder>()).Get();
  _rootFolder->SetName("Root");

  _theatre->Clear();
}

void Management::UpdateUniverses() {
  std::lock_guard<std::mutex> lock(_mutex);
  const size_t n_universes = universe_map_.NUniverses();
  _primarySnapshot.SetUniverseCount(n_universes);
  _secondarySnapshot.SetUniverseCount(n_universes);
}

void Management::Run() {
  if (_thread == nullptr) {
    UpdateUniverses();
    _isQuitting = false;
    _thread = std::make_unique<std::thread>([&]() { ThreadLoop(); });
  } else
    throw std::runtime_error("Invalid call to Run(): already running");
}

void Management::InferInputUniverse(unsigned universe, ValueSnapshot &snapshot,
                                    bool is_primary) {
  unsigned values[kChannelsPerUniverse];

  std::fill_n(values, kChannelsPerUniverse, 0);

  for (const TrackablePtr<Controllable> &controllable : _controllables) {
    if (FixtureControl *fc =
            dynamic_cast<FixtureControl *>(controllable.Get())) {
      fc->GetChannelValues(values, universe);
    }
  }

  unsigned char values_char[kChannelsPerUniverse];
  for (unsigned i = 0; i < kChannelsPerUniverse; ++i) {
    unsigned val = (values[i] >> 16);
    if (val > 255) val = 255;
    values_char[i] = static_cast<unsigned char>(val);
  }

  ValueUniverseSnapshot &universe_values =
      snapshot.GetUniverseSnapshot(universe);
  universe_values.SetValues(values_char, _theatre->HighestChannel() + 1);
}

void Management::MergeInputUniverse(ValueSnapshot &snapshot,
                                    size_t input_universe) {
  const system::OptionalNumber<size_t> destination_universe =
      universe_map_.GetInputMapping(input_universe).merge_universe;
  if (destination_universe &&
      *destination_universe < snapshot.UniverseCount()) {
    unsigned char values[kChannelsPerUniverse];
    universe_map_.GetInputValues(input_universe, values, kChannelsPerUniverse);
    ValueUniverseSnapshot &universe_snapshot =
        snapshot.GetUniverseSnapshot(*destination_universe);
    for (size_t ch = 0; ch != kChannelsPerUniverse; ++ch) {
      universe_snapshot[ch] = std::max(universe_snapshot[ch], values[ch]);
    }
  }
}

void Management::ThreadLoop() {
  const size_t n_universes = universe_map_.NUniverses();
  ValueSnapshot next_primary(true, n_universes);
  ValueSnapshot next_secondary(false, n_universes);
  unsigned timestep_number = 0;
  while (!_isQuitting) {
    MixAll(timestep_number, next_primary, next_secondary);
    universe_map_.WaitForNextSync();

    std::lock_guard<std::mutex> lock(_mutex);
    swap(_primarySnapshot, next_primary);
    swap(_secondarySnapshot, next_secondary);

    ++timestep_number;
  }
}

void Management::abortAllDevices() { universe_map_.Close(); }

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
  for (const TrackablePtr<Controllable> &c : _controllables)
    unorderedList.emplace_back(c.Get());
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
    const unsigned n_universes = universe_map_.NUniverses();
    ValueSnapshot &snapshot = is_primary ? primary : secondary;
    for (unsigned universe = 0; universe != n_universes; ++universe) {
      if (universe_map_.GetUniverseType(universe) == UniverseType::Output) {
        InferInputUniverse(universe, snapshot, is_primary);
      }
    }

    // Merge any input universes that are set to be merged
    if (is_primary) {
      for (unsigned universe = 0; universe != n_universes; ++universe) {
        if (universe_map_.GetUniverseType(universe) == UniverseType::Input &&
            universe_map_.GetInputMapping(universe).function ==
                devices::InputMappingFunction::Merge) {
          MergeInputUniverse(snapshot, universe);
        }
      }

      // Output universes
      for (unsigned universe = 0; universe != n_universes; ++universe) {
        if (universe_map_.GetUniverseType(universe) == UniverseType::Output) {
          universe_map_.SetOutputValues(
              universe, snapshot.GetUniverseSnapshot(universe).Data(),
              kChannelsPerUniverse);
        }
      }
    }
  }
}

bool Management::HasCycle() const {
  std::vector<Controllable *> unorderedList;
  std::vector<Controllable *> orderedList;
  for (const TrackablePtr<Controllable> &c : _controllables)
    unorderedList.emplace_back(c.Get());
  return !topologicalSort(unorderedList, orderedList);
}

const TrackablePtr<Controllable> &Management::AddPresetCollection() {
  return _controllables.emplace_back(
      TrackablePtr<Controllable>(new PresetCollection()));
}

Folder &Management::AddFolder(Folder &parent, const std::string &name) {
  _folders.emplace_back(MakeTrackable<Folder>(name));
  parent.Add(_folders.back().GetObserver());
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
    if (iter->Get() == &folder) {
      _folders.erase(iter);
      break;
    }
  }
}

void Management::RemoveControllable(Controllable &controllable) {
  removeControllable(_controllables.begin() +
                     NamedObject::FindIndex(_controllables, &controllable));
}

void Management::removeControllable(
    std::vector<TrackablePtr<Controllable>>::iterator controllablePtr) {
  TrackablePtr<Controllable> controllable = std::move(*controllablePtr);

  _controllables.erase(controllablePtr);

  auto result =
      std::remove_if(_sourceValues.begin(), _sourceValues.end(),
                     [&controllable](std::unique_ptr<SourceValue> &pv) {
                       return &pv->GetControllable() == controllable.Get();
                     });
  _sourceValues.erase(result, _sourceValues.end());

  controllable->Parent().Remove(*controllable);

  std::vector<TrackablePtr<Controllable>>::iterator i = _controllables.begin();
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

bool Management::Contains(const Controllable &controllable) const {
  for (const TrackablePtr<Controllable> &contr : _controllables) {
    if (contr.Get() == &controllable) return true;
  }
  return false;
}

const TrackablePtr<Controllable> &Management::AddFixtureControl(
    const Fixture &fixture) {
  return _controllables.emplace_back(TrackablePtr<Controllable>(
      new FixtureControl(const_cast<Fixture &>(fixture))));
}

const TrackablePtr<Controllable> &Management::AddFixtureControl(
    const Fixture &fixture, const Folder &parent) {
  const TrackablePtr<Controllable> &fixture_control =
      _controllables.emplace_back(TrackablePtr<Controllable>(
          new FixtureControl(const_cast<Fixture &>(fixture))));
  const_cast<Folder &>(parent).Add(fixture_control.GetObserver());
  return fixture_control;
}

ObservingPtr<FixtureControl> Management::GetFixtureControl(
    const Fixture &fixture) const {
  for (const TrackablePtr<Controllable> &contr : _controllables) {
    FixtureControl *fc = dynamic_cast<FixtureControl *>(contr.Get());
    if (fc) {
      if (&fc->GetFixture() == &fixture) return contr.GetObserver();
    }
  }
  throw std::runtime_error("GetFixtureControl() : Fixture control not found");
}

void Management::RemoveFixture(const Fixture &fixture) {
  FixtureControl &control = *GetFixtureControl(fixture);
  _theatre->RemoveFixture(fixture);
  RemoveControllable(control);
}

void Management::RemoveFixtureType(const FixtureType &fixtureType) {
  const std::vector<system::TrackablePtr<Fixture>> &fixtures =
      _theatre->Fixtures();
  size_t i = 0;
  while (i != fixtures.size()) {
    // Go backward through the list, as fixtures might be removed
    const size_t fIndex = fixtures.size() - 1 - i;
    Fixture &f = *fixtures[fIndex];
    if (&f.Type() == &fixtureType) {
      RemoveFixture(*fixtures[fIndex]);
    } else {
      ++i;
    }
  }
  _theatre->RemoveFixtureType(fixtureType);
}

const TrackablePtr<FixtureGroup> &Management::AddFixtureGroup() {
  return _groups.emplace_back(MakeTrackable<FixtureGroup>());
}

const TrackablePtr<FixtureGroup> &Management::AddFixtureGroup(
    const Folder &parent, const std::string &name) {
  const TrackablePtr<FixtureGroup> &group =
      _groups.emplace_back(MakeTrackable<FixtureGroup>(name));
  const_cast<Folder &>(parent).Add(group.GetObserver());
  return group;
}

void Management::RemoveFixtureGroup(const FixtureGroup &group) {
  for (std::vector<TrackablePtr<FixtureGroup>>::iterator i = _groups.begin();
       i != _groups.end(); ++i) {
    if (i->Get() == &group) {
      i->Get()->Parent().Remove(**i);
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

bool Management::Contains(const SourceValue &sourceValue) const {
  for (const std::unique_ptr<SourceValue> &sv : _sourceValues) {
    if (sv.get() == &sourceValue) return true;
  }
  return false;
}

const TrackablePtr<Controllable> &Management::AddChase() {
  return _controllables.emplace_back(TrackablePtr<Controllable>(new Chase()));
}

const TrackablePtr<Controllable> &Management::AddTimeSequence() {
  return _controllables.emplace_back(
      TrackablePtr<Controllable>(new TimeSequence()));
}

const TrackablePtr<Controllable> &Management::AddEffect(
    std::unique_ptr<Effect> effect) {
  return _controllables.emplace_back(
      TrackablePtr<Controllable>(std::move(effect)));
}

system::ObservingPtr<Effect> Management::AddEffectPtr(
    std::unique_ptr<Effect> effect) {
  return AddEffect(std::move(effect)).GetObserver<Effect>();
}

const TrackablePtr<Controllable> &Management::AddEffect(
    std::unique_ptr<Effect> effect, Folder &folder) {
  const TrackablePtr<Controllable> &newEffect = AddEffect(std::move(effect));
  folder.Add(newEffect.GetObserver());
  return newEffect;
}

system::ObservingPtr<Effect> Management::AddEffectPtr(
    std::unique_ptr<Effect> effect, Folder &folder) {
  return AddEffect(std::move(effect), folder).GetObserver<Effect>();
}

const TrackablePtr<Controllable> &Management::AddScene(bool in_folder) {
  const TrackablePtr<Controllable> &result =
      _controllables.emplace_back(TrackablePtr<Controllable>(new Scene(*this)));
  if (in_folder) {
    result->SetName(_rootFolder->GetAvailableName("scene"));
    _rootFolder->Add(result.GetObserver());
  }
  return result;
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

ValueSnapshot Management::PrimarySnapshot() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _primarySnapshot;
}

ValueSnapshot Management::SecondarySnapshot() const {
  std::lock_guard<std::mutex> lock(_mutex);
  return _secondarySnapshot;
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
