#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include <atomic>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "valuesnapshot.h"

class Controllable;
class DmxDevice;
class Folder;
class PresetValue;
class SourceValue;

/**
 * @author Andre Offringa
 */
class Management {
 public:
  Management();
  ~Management();

  void Clear();

  bool IsEmpty() {
    return _folders.size() <= 1 && _controllables.empty() &&
           _sourceValues.empty();
  }

  void AddDevice(std::unique_ptr<class DmxDevice> device);

  void Run();

  void StartBeatFinder();

  class Theatre &Theatre() const {
    return *_theatre;
  }

  const std::vector<std::unique_ptr<class Folder>> &Folders() const {
    return _folders;
  }
  const std::vector<std::unique_ptr<class Controllable>> &Controllables()
      const {
    return _controllables;
  }
  const std::vector<std::unique_ptr<class SourceValue>> &SourceValues() const {
    return _sourceValues;
  }
  const std::vector<std::unique_ptr<class DmxDevice>> &Devices() const {
    return _devices;
  }

  void RemoveObject(class FolderObject &object);

  class PresetCollection &AddPresetCollection();
  void RemoveControllable(class Controllable &controllable);
  bool Contains(class Controllable &controllable) const;

  class Folder &AddFolder(class Folder &parent, const std::string &name);
  class Folder &GetFolder(const std::string &path);
  void RemoveFolder(class Folder &folder);

  class FixtureControl &AddFixtureControl(class Fixture &fixture);
  class FixtureControl &AddFixtureControl(class Fixture &fixture,
                                          Folder &parent);
  class FixtureControl &GetFixtureControl(class Fixture &fixture);

  void RemoveFixture(class Fixture &fixture);

  class SourceValue &AddSourceValue(Controllable &controllable,
                                    size_t inputIndex);

  void RemoveSourceValue(class SourceValue &sourceValue);
  bool Contains(class SourceValue &sourceValue) const;

  class Chase &AddChase();

  class TimeSequence &AddTimeSequence();

  class Effect &AddEffect(std::unique_ptr<class Effect> effect);
  class Effect &AddEffect(std::unique_ptr<class Effect> effect, Folder &folder);

  std::mutex &Mutex() { return _mutex; }

  class FolderObject &GetObjectFromPath(const std::string &path) const;
  class FolderObject *GetObjectFromPathIfExists(const std::string &path) const;
  size_t ControllableIndex(const Controllable *controllable) const;

  class SourceValue *GetSourceValue(Controllable &controllable,
                                    size_t inputIndex) const;
  size_t SourceValueIndex(const class SourceValue *sourceValue) const;
  class ValueSnapshot Snapshot();

  double GetOffsetTimeInMS() const {
    boost::posix_time::ptime currentTime(
        boost::posix_time::microsec_clock::local_time());
    return (double)(currentTime - _createTime).total_microseconds() / 1000.0;
  }
  class Show &Show() const {
    return *_show;
  }

  std::unique_ptr<Management> MakeDryMode();

  /**
   * Swap DMX devices of two managements.
   *
   * This can be called while running, and is e.g. useful for switching from dry
   * mode.
   */
  void SwapDevices(Management &source);

  const class Folder &RootFolder() const { return *_rootFolder; }
  class Folder &RootFolder() {
    return *_rootFolder;
  }

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

  void Recover(Management &other);

 private:
  struct ManagementThread {
    Management *parent;
    void operator()();
  };

  Management(const Management &forDryCopy,
             std::shared_ptr<class BeatFinder> &beatFinder);

  void getChannelValues(unsigned timestepNumber, unsigned *values,
                        unsigned universe);
  void removeControllable(
      std::vector<std::unique_ptr<class Controllable>>::iterator
          controllablePtr);

  void dryCopyControllerDependency(const Management &forDryCopy, size_t index);
  void dryCopyEffectDependency(const Management &forDryCopy, size_t index);

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
  boost::posix_time::ptime _createTime;
  std::mt19937 _randomGenerator;
  std::uniform_int_distribution<unsigned> _rndDistribution;
  std::atomic<size_t> _overridenBeat;
  std::atomic<double> _lastOverridenBeatTime;
  std::atomic<double> _previousTime;

  std::unique_ptr<class Theatre> _theatre;
  std::unique_ptr<class ValueSnapshot> _snapshot;
  std::shared_ptr<class BeatFinder> _beatFinder;
  std::unique_ptr<class Show> _show;

  Folder *_rootFolder;
  std::vector<std::unique_ptr<Folder>> _folders;
  std::vector<std::unique_ptr<Controllable>> _controllables;
  std::vector<std::unique_ptr<SourceValue>> _sourceValues;
  std::vector<std::unique_ptr<DmxDevice>> _devices;
};

#endif
