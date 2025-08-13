#ifndef THEATRE_MANAGEMENT_H_
#define THEATRE_MANAGEMENT_H_

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#include "forwards.h"
#include "valuesnapshot.h"
#include "sourcevaluestore.h"

#include "devices/universemap.h"

namespace glight::system {
struct Settings;
}

namespace glight::theatre {

/**
 * @author Andre Offringa
 */
class Management {
 public:
  Management(const system::Settings &settings);
  ~Management();

  void Clear();

  bool IsEmpty() const {
    return _folders.size() <= 1 && _controllables.empty() &&
           _sourceValues.empty();
  }

  void UpdateUniverses();

  void Run();

  /**
   * Start or restart the beat finder. This is not done automatically because
   * for testing and certain other scenarios it might not be desirable to access
   * the audio device.
   *
   * If the beat finder is running, calling this function again can be used to
   * use a new device after the device has been changed in the settings.
   */
  void StartBeatFinder();
  BeatFinder &GetBeatFinder() { return *_beatFinder; }

  Theatre &GetTheatre() { return *_theatre; }
  const Theatre &GetTheatre() const { return *_theatre; }

  const std::vector<system::TrackablePtr<Folder>> &Folders() const {
    return _folders;
  }
  const std::vector<system::TrackablePtr<FixtureGroup>> &FixtureGroups() const {
    return _groups;
  }
  const std::vector<system::TrackablePtr<Controllable>> &Controllables() const {
    return _controllables;
  }
  const std::vector<std::unique_ptr<SourceValue>> &SourceValues() const {
    return _sourceValues;
  }
  std::vector<std::unique_ptr<SourceValue>> &SourceValues() {
    return _sourceValues;
  }
  devices::UniverseMap &GetUniverses() { return universe_map_; }

  void RemoveObject(FolderObject &object);

  const system::TrackablePtr<Controllable> &AddPresetCollection();
  system::ObservingPtr<PresetCollection> AddPresetCollectionPtr();

  void RemoveControllable(Controllable &controllable);
  bool Contains(const Controllable &controllable) const;
  template <typename ControllableType>
  /**
   * Retrieve list of controllables of a specific type. Can for example
   * be used to get a list of variables.
   */
  std::vector<ControllableType *> GetSpecificControllables() const {
    std::vector<ControllableType *> list;
    for (const system::TrackablePtr<Controllable> &c : _controllables) {
      if (ControllableType *t = dynamic_cast<ControllableType *>(c.Get()); t) {
        list.emplace_back(t);
      }
    }
    return list;
  }

  Folder &AddFolder(Folder &parent, const std::string &name);
  Folder &GetFolder(const std::string &path);
  void RemoveFolder(Folder &folder);

  const system::TrackablePtr<Controllable> &AddFixtureControl(
      const Fixture &fixture);
  system::ObservingPtr<FixtureControl> AddFixtureControlPtr(
      const Fixture &fixture);

  const system::TrackablePtr<Controllable> &AddFixtureControl(
      const Fixture &fixture, const Folder &parent);
  system::ObservingPtr<FixtureControl> AddFixtureControlPtr(
      const Fixture &fixture, const Folder &parent);

  system::ObservingPtr<FixtureControl> GetFixtureControl(
      const Fixture &fixture) const;

  const system::TrackablePtr<FixtureGroup> &AddFixtureGroup();
  const system::TrackablePtr<FixtureGroup> &AddFixtureGroup(
      const Folder &parent, const std::string &name);

  void RemoveFixture(const Fixture &fixture);
  void RemoveFixtureType(const FixtureType &type);
  void RemoveFixtureGroup(const FixtureGroup &group);

  SourceValue &AddSourceValue(Controllable &controllable, size_t inputIndex);

  void RemoveSourceValue(SourceValue &sourceValue);
  bool Contains(const SourceValue &sourceValue) const;

  const system::TrackablePtr<Controllable> &AddChase();
  system::ObservingPtr<Chase> AddChasePtr();

  const system::TrackablePtr<Controllable> &AddTimeSequence();
  system::ObservingPtr<TimeSequence> AddTimeSequencePtr();

  /**
   * Add an effect and do not place it in a folder. The caller needs to
   * manually place the effect in a folder.
   */
  const system::TrackablePtr<Controllable> &AddEffect(
      std::unique_ptr<Effect> effect);
  system::ObservingPtr<Effect> AddEffectPtr(std::unique_ptr<Effect> effect);

  /**
   * Add an effect and place it in a folder.
   */
  const system::TrackablePtr<Controllable> &AddEffect(
      std::unique_ptr<Effect> effect, Folder &folder);
  system::ObservingPtr<Effect> AddEffectPtr(std::unique_ptr<Effect> effect,
                                            Folder &folder);

  const system::TrackablePtr<Controllable> &AddScene(bool in_folder);
  system::ObservingPtr<Scene> AddScenePtr(bool in_folder);

  std::mutex &Mutex() { return _mutex; }

  const system::Settings &Settings() const { return settings_; }

  FolderObject &GetObjectFromPath(const std::string &path) const;

  FolderObject *GetObjectFromPathIfExists(const std::string &path) const;
  size_t ControllableIndex(const Controllable *controllable) const;

  SourceValue *GetSourceValue(const Controllable &controllable,
                              size_t input_index);
  const SourceValue *GetSourceValue(const Controllable &controllable,
                                    size_t input_index) const {
    return const_cast<Management &>(*this).GetSourceValue(controllable,
                                                          input_index);
  }
  size_t SourceValueIndex(const SourceValue *sourceValue) const;
  ValueSnapshot Snapshot(bool primary);
  ValueSnapshot PrimarySnapshot() const;
  ValueSnapshot SecondarySnapshot() const;

  double GetOffsetTimeInMS() const {
    const std::chrono::time_point<std::chrono::steady_clock> current_time =
        std::chrono::steady_clock::now();
    const std::chrono::duration<double> d = current_time - _createTime;
    return std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
  }

  const Folder &RootFolder() const { return *_rootFolder; }
  Folder &RootFolder() { return *_rootFolder; }

  bool HasCycle() const;

  void IncreaseManualBeat(unsigned count = 1) {
    if (count == 0) {
      _lastOverridenBeatTime = 0.0;
    } else {
      _overridenBeat += count;
      _lastOverridenBeatTime = GetOffsetTimeInMS();
    }
  }

  void BlackOut(bool skip_scenes, double fade_speed);

  /**
   * Returns a source value store with all values as currently set in
   * the primary (A) or secondary (B) values. The store will not include
   * source values associated with a Scene.
   */
  SourceValueStore StoreSourceValues(bool use_a) const;

  void LoadSourceValues(const SourceValueStore &store, bool use_a,
                        double fade_speed);

 private:
  void ThreadLoop();

  /**
   * Prepares the dependency chain, and propagates values starting at the
   * source values through the controllables.
   */
  void MixAll(unsigned timestep_number, ValueSnapshot &primary,
              ValueSnapshot &secondary);

  /**
   * Obtains the channel values from the current situation of the controllables.
   * If this is the primary snapshot, the values are also send to the DMX
   * device.
   */
  void InferInputUniverse(unsigned universe, ValueSnapshot &snapshot,
                          bool is_primary);

  void MergeInputUniverse(ValueSnapshot &snapshot, size_t input_universe);

  void removeControllable(
      std::vector<system::TrackablePtr<Controllable>>::iterator
          controllablePtr);

  void abortAllDevices();

  /**
   * Sorts controllables such that when A outputs to B, then A will come
   * after B in the ordered list.
   */
  static bool topologicalSort(const std::vector<Controllable *> &input,
                              std::vector<Controllable *> &output);
  static bool topologicalSortVisit(Controllable &controllable,
                                   std::vector<Controllable *> &list);

  std::unique_ptr<std::thread> _thread;
  std::atomic<bool> _isQuitting = false;
  mutable std::mutex _mutex;
  const system::Settings &settings_;
  std::chrono::time_point<std::chrono::steady_clock> _createTime =
      std::chrono::steady_clock::now();
  std::mt19937 _randomGenerator;
  std::uniform_int_distribution<unsigned> _rndDistribution =
      std::uniform_int_distribution<unsigned>(0, ControlValue::MaxUInt() + 1);
  std::atomic<size_t> _overridenBeat = 0;
  std::atomic<double> _lastOverridenBeatTime = 0.0;
  std::atomic<double> _previousTime = 0.0;

  std::unique_ptr<Theatre> _theatre;
  ValueSnapshot _primarySnapshot = {true, 0};
  ValueSnapshot _secondarySnapshot = {false, 0};
  std::unique_ptr<BeatFinder> _beatFinder;

  Folder *_rootFolder;
  std::vector<system::TrackablePtr<Folder>> _folders;
  std::vector<system::TrackablePtr<Controllable>> _controllables;
  std::vector<system::TrackablePtr<FixtureGroup>> _groups;
  std::vector<std::unique_ptr<SourceValue>> _sourceValues;
  devices::UniverseMap universe_map_;
};

}  // namespace glight::theatre

#endif
