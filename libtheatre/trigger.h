#ifndef TRIGGER_H
#define TRIGGER_H

#include <string>

/**
	@author Andre Offringa
*/
class Trigger {
	public:
		enum Type { DelayTriggered, SyncTriggered, BeatTriggered };

		Trigger() :
			_type(DelayTriggered),
			_delayInMs(500.0),
			_delaySynced(1),
			_delayInBeats(1.0)
		{ }

		double DelayInMs() const { return _delayInMs; }
		void SetDelayInMs(double delay) { _delayInMs = delay; }
		
		unsigned DelayInSyncs() const { return _delaySynced; }
		void SetDelayInSyncs(unsigned syncs) { _delaySynced = syncs; }

		double DelayInBeats() const { return _delayInBeats; }
		void SetDelayInBeats(double delay) { _delayInBeats = delay; }
		
		enum Type Type() const { return _type; }
		void SetType(enum Type type) { _type = type; }
		
		std::string ToString() const
		{
			switch(_type)
			{
				case DelayTriggered:
					return std::to_string(_delayInMs/1000.0) + " s";
				case SyncTriggered:
					return std::to_string(_delaySynced) + " syncs";
				case BeatTriggered:
					return std::to_string(_delayInBeats) + " beats";
			}
			return std::string();
		}
		
	private:
		enum Type _type;
		double _delayInMs;
		unsigned _delaySynced;
		double _delayInBeats;
};

#endif
