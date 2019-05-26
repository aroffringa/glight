#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include <vector>
#include <thread>
#include <mutex>
#include <random>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "valuesnapshot.h"

/**
	@author Andre Offringa
*/
class Management {
	public:
		Management();
		~Management();
		
		void Clear();

		void AddDevice(std::unique_ptr<class DmxDevice> device);
		
		void Run();
		
		void StartBeatFinder();
		
		class Theatre &Theatre() const { return *_theatre; }

		const std::vector<std::unique_ptr<class Folder>>& Folders() const
		{
			return _folders;
		}
		const std::vector<std::unique_ptr<class Controllable>>& Controllables() const
		{
			return _controllables;
		}
		const std::vector<std::unique_ptr<class PresetValue>>& PresetValues() const
		{
			return _presetValues;
		}
		const std::vector<std::unique_ptr<class DmxDevice>>& Devices() const
		{
			return _devices;
		}
		
		void RemoveObject(class FolderObject& object);

		class PresetCollection& AddPresetCollection();
		void RemoveControllable(class Controllable &controllable);
		bool Contains(class Controllable &controllable) const;
		
		class Folder& AddFolder(class Folder& parent);
		class Folder& GetFolder(const std::string& path);
		void RemoveFolder(class Folder& folder);

		class FixtureControl& AddFixtureControl(class Fixture& fixture);
		class FixtureControl& AddFixtureControl(class Fixture& fixture, Folder& parent);
		class FixtureControl& GetFixtureControl(class Fixture& fixture);
		
		void RemoveFixture(class Fixture& fixture);

		class PresetValue& AddPreset(Controllable &controllable, size_t inputIndex);
		class PresetValue& AddPreset(unsigned id, Controllable &controllable, size_t inputIndex);

		void RemovePreset(class PresetValue &presetValue);
		bool Contains(class PresetValue &controllable) const;

		class Chase& AddChase();
		
		class Effect& AddEffect(std::unique_ptr<class Effect> effect);
		class Effect& AddEffect(std::unique_ptr<class Effect> effect, Folder& folder);

		std::mutex& Mutex() { return _mutex; }

		class Controllable& GetControllable(const std::string& name) const;
		class FolderObject& GetObjectFromPath(const std::string& path) const;
		size_t ControllableIndex(const Controllable* controllable) const;
		
		class PresetValue* GetPresetValue(unsigned id) const;
		class PresetValue* GetPresetValue(Controllable& controllable, size_t inputIndex) const;
		size_t PresetValueIndex(const class PresetValue* presetValue) const;
		class ValueSnapshot Snapshot();
		
		double GetOffsetTimeInMS() const
		{
			boost::posix_time::ptime currentTime(boost::posix_time::microsec_clock::local_time());
			return (double) (currentTime - _createTime).total_microseconds() / 1000.0;
		}
		class Show& Show() const
		{
			return *_show;
		}
		
		std::unique_ptr<Management> MakeDryMode();
		
		/**
		 * Swap DMX devices of two managements.
		 * 
		 * This can be called while running, and is e.g. useful for switching from dry mode.
		 */
		void SwapDevices(Management& source);
		
		const class Folder& RootFolder() const { return *_rootFolder; }
		class Folder& RootFolder() { return *_rootFolder; }
		
	private:
		struct ManagementThread {
			Management *parent;
			void operator()();
		};
		
		Management(const Management& forDryCopy, std::shared_ptr<class BeatFinder>& beatFinder);

		void getChannelValues(unsigned timestepNumber, unsigned *values, unsigned universe);
		void removeControllable(std::vector<std::unique_ptr<class Controllable>>::iterator controllablePtr);
		void removePreset(std::vector<std::unique_ptr<class PresetValue>>::iterator presetValuePtr);
		
		void dryCopyControllerDependency(const Management& forDryCopy, size_t index);
		void dryCopyEffectDependency(const Management& forDryCopy, size_t index);

		void abortAllDevices();
		
		/**
		 * Sorts controllables such that when A outputs to B, then A will come
		 * after B in the ordered list.
		 */
		static void topologicalSort(const std::vector<Controllable*>& input, std::vector<Controllable*>& output);
		static void topologicalSortVisit(Controllable& controllable, std::vector<Controllable*>& list);

		std::unique_ptr<std::thread> _thread;
		std::atomic<bool> _isQuitting;
		std::mutex _mutex;
		boost::posix_time::ptime _createTime;
		std::mt19937 _randomGenerator;
		std::uniform_int_distribution<unsigned> _rndDistribution;
		unsigned _nextPresetValueId;

		std::unique_ptr<class Theatre> _theatre;
		std::unique_ptr<class ValueSnapshot> _snapshot;
		std::shared_ptr<class BeatFinder> _beatFinder;
		std::unique_ptr<class Show> _show;

		class Folder* _rootFolder;
		std::vector<std::unique_ptr<class Folder>> _folders;
		std::vector<std::unique_ptr<class Controllable>> _controllables;
		std::vector<std::unique_ptr<class PresetValue>> _presetValues;
		std::vector<std::unique_ptr<class DmxDevice>> _devices;
};

#endif
