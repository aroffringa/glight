#ifndef TRIGGER_H
#define TRIGGER_H

/**
	@author Andre Offringa
*/
class Trigger {
	public:
		enum Type { DelayTriggered, BeatTriggered };

		Trigger()
			: _type(DelayTriggered), _delayInMs(500.0), _delayInBeats(1.0)
		{
		}
		~Trigger()
		{
		}
		double DelayInMs() const { return _delayInMs; }
		void SetDelayInMs(double delay) { _delayInMs = delay; }

		double DelayInBeats() const { return _delayInBeats; }
		void SetDelayInBeats(double delay) { _delayInBeats = delay; }
		
		enum Type Type() const { return _type; }
		void SetType(enum Type type) { _type = type; }
	private:
		enum Type _type;
		double _delayInMs, _delayInBeats;
};

#endif
