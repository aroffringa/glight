#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include <vector>
#include <thread>
#include <mutex>

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
		
		class Theatre &Theatre() const { return *_theatre; }

		const std::vector<std::unique_ptr<class Controllable>>& Controllables() const
		{
			return _controllables;
		}
		const std::vector<std::unique_ptr<class PresetValue>>& PresetValues() const
		{
			return _presetValues;
		}
		const std::vector<std::unique_ptr<class Sequence>>& Sequences() const
		{
			return _sequences;
		}
		const std::vector<std::unique_ptr<class DmxDevice>>& Devices() const
		{
			return _devices;
		}

		class PresetCollection &AddPresetCollection();
		void RemoveControllable(class Controllable &controllable);
		bool Contains(class Controllable &controllable) const;

		class FixtureFunctionControl& AddFixtureFunctionControl(class FixtureFunction &function);

		class PresetValue& AddPreset(Controllable &controllable);
		class PresetValue& AddPreset(unsigned id, Controllable &controllable);

		void RemovePreset(class PresetValue &presetValue);
		bool Contains(class PresetValue &controllable) const;

		class Sequence& AddSequence();
		void RemoveSequence(class Sequence &sequence);

		class Chase& AddChase(class Sequence &sequence);

		std::mutex& Mutex() { return _mutex; }

		class Controllable& GetControllable(const std::string &name) const;
		class Sequence& GetSequence(const std::string &name) const;
		class PresetValue& GetPresetValue(unsigned id) const;
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
	private:
		struct ManagementThread {
			Management *parent;
			void operator()();
		};
		
		Management(const Management& forDryCopy, std::shared_ptr<class BeatFinder>& beatFinder);

		void GetChannelValues(unsigned *values, unsigned universe);
		void removeControllable(std::vector<std::unique_ptr<class Controllable>>::iterator controllablePtr);
		void removePreset(std::vector<std::unique_ptr<class PresetValue>>::iterator presetValuePtr);
		void removeSequence(std::vector<std::unique_ptr<class Sequence>>::iterator sequencePtr);

		bool IsQuitting()
		{
			std::lock_guard<std::mutex> lock(_mutex);
			return _isQuitting;
		}

		void Quit()
		{
			std::unique_lock<std::mutex> lock(_mutex);
			_isQuitting = true;
			lock.unlock();

			AbortAllDevices();
		}
		void AbortAllDevices();

		std::unique_ptr<std::thread> _thread;
		bool _isRunning, _isQuitting;
		std::mutex _mutex;
		boost::posix_time::ptime _createTime;
		unsigned _nextPresetValueId;

		std::unique_ptr<class Theatre> _theatre;
		std::unique_ptr<class ValueSnapshot> _snapshot;
		std::shared_ptr<class BeatFinder> _beatFinder;
		std::unique_ptr<class Show> _show;

		std::vector<std::unique_ptr<class Controllable>> _controllables;
		std::vector<std::unique_ptr<class PresetValue>> _presetValues;
		std::vector<std::unique_ptr<class Sequence>> _sequences;
		std::vector<std::unique_ptr<class DmxDevice>> _devices;
};

#endif
