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

namespace glight::theatre {

/**
 * @author Andre Offringa
 */
class Management {
 public:
  Management();
  ~Management();

  void Clear();

  bool IsEmpty() const {
    return _folders.size() <= 1 && _controllables.empty() &&
           _sourceValues.empty();
  }

  void AddDevice(std::unique_ptr<DmxDevice> device);

  void Run();

  void StartBeatFinder();

  Theatre &GetTheatre() { return *_theatre; }
  const Theatre &GetTheatre() const { return *_theatre; }

  const std::vector<std::unique_ptr<Folder>> &Folders() const {
    return _folders;
  }
  const std::vector<std::unique_ptr<FixtureGroup>> &FixtureGroups() const {
    return _groups;
  }
  const std::vector<std::unique_ptr<Controllable>> &Controllables() const {
    return _controllables;
  }
  const std::vector<std::unique_ptr<SourceValue>> &SourceValues() const {
    return _sourceValues;
  }
  std::vector<std::unique_ptr<SourceValue>> &SourceValues() {
    return _sourceValues;
  }
  const std::vector<std::unique_ptr<DmxDevice>> &Devices() const {
    return _devices;
  }

  void RemoveObject(FolderObject &object);

  PresetCollection &AddPresetCollection();
  void RemoveControllable(Controllable &controllable);
  bool Contains(Controllable &controllable) const;

  Folder &AddFolder(Folder &parent, const std::string &name);
  Folder &GetFolder(const std::string &path);
  void RemoveFolder(Folder &folder);

  FixtureControl &AddFixtureControl(const Fixture &fixture);
  FixtureControl &AddFixtureControl(const Fixture &fixture,
                                    const Folder &parent);
  FixtureControl &GetFixtureControl(const Fixture &fixture);
  const FixtureControl &GetFixtureControl(const Fixture &fixture) const {
    return const_cast<Management &>(*this).GetFixtureControl(fixture);
  }

  FixtureGroup &AddFixtureGroup();
  FixtureGroup &AddFixtureGroup(const Folder &parent, const std::string &name);

  void RemoveFixture(const Fixture &fixture);
  void RemoveFixtureType(const FixtureType &type);
  void RemoveFixtureGroup(const FixtureGroup &group);

  SourceValue &AddSourceValue(Controllable &controllable, size_t inputIndex);

  void RemoveSourceValue(SourceValue &sourceValue);
  bool Contains(SourceValue &sourceValue) const;

  Chase &AddChase();

  TimeSequence &AddTimeSequence();

  Effect &AddEffect(std::unique_ptr<Effect> effect);
  Effect &AddEffect(std::unique_ptr<Effect> effect, Folder &folder);

  Scene &AddScene(bool in_folder);

  std::mutex &Mutex() { return _mutex; }

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
  ValueSnapshot PrimarySnapshot();
  ValueSnapshot SecondarySnapshot();

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

  void BlackOut();

 private:
  void ThreadLoop();

  void getChannelValues(unsigned timestepNumber, unsigned *values,
                        unsigned universe, bool primary);
  void removeControllable(
      std::vector<std::unique_ptr<Controllable>>::iterator controllablePtr);

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
  std::atomic<bool> _isQuitting;
  std::mutex _mutex;
  std::chrono::time_point<std::chrono::steady_clock> _createTime;
  std::mt19937 _randomGenerator;
  std::uniform_int_distribution<unsigned> _rndDistribution;
  std::atomic<size_t> _overridenBeat;
  std::atomic<double> _lastOverridenBeatTime;
  std::atomic<double> _previousTime;

  std::unique_ptr<Theatre> _theatre;
  std::unique_ptr<ValueSnapshot> _primarySnapshot;
  std::unique_ptr<ValueSnapshot> _secondarySnapshot;
  std::shared_ptr<BeatFinder> _beatFinder;

  Folder *_rootFolder;
  std::vector<std::unique_ptr<Folder>> _folders;
  std::vector<std::unique_ptr<Controllable>> _controllables;
  std::vector<std::unique_ptr<FixtureGroup>> _groups;
  std::vector<std::unique_ptr<SourceValue>> _sourceValues;
  std::vector<std::unique_ptr<DmxDevice>> _devices;
};

}  // namespace glight::theatre

#endif
