#ifndef CHASE_H
#define CHASE_H

#include "controllable.h"
#include "sequence.h"
#include "timing.h"
#include "trigger.h"
#include "transition.h"

/**
	@author Andre Offringa
*/
class Chase : public Controllable {
public:
	Chase()
	{ }
	
	Chase(const Chase& chase) :
		Controllable(chase),
		_sequence(chase._sequence),
		_trigger(chase._trigger),
		_transition(chase._transition)
	{ }

	size_t NInputs() const final override
	{ return 1; }
	
	ControlValue& InputValue(size_t) final override
	{ return _inputValue; }
	
	size_t NOutputs() const final override
	{ return _sequence.List().size(); }
	
	std::pair<Controllable*, size_t> Output(size_t index) const final override
	{ return std::make_pair(_sequence.List()[index], 0); }
	
	virtual void Mix(unsigned *channelValues, unsigned universe, const Timing& timing) final override
	{
		switch(_trigger.Type()) {
		case Trigger::DelayTriggered:
			mixDelayChase(_inputValue, channelValues, universe, timing);
			break;
		case Trigger::SyncTriggered:
			mixSyncedChase(_inputValue, channelValues, universe, timing);
			break;
		case Trigger::BeatTriggered:
			mixBeatChase(_inputValue, channelValues, universe, timing);
			break;
		}
	}
	
	const class Transition& Transition() const { return _transition; }
	class Transition& Transition() { return _transition; }

	const class Trigger& Trigger() const { return _trigger; }
	class Trigger& Trigger() { return _trigger; }

	const class Sequence& Sequence() const { return _sequence; }
	class Sequence& Sequence() { return _sequence; }

private:
	void mixBeatChase(const ControlValue& value, unsigned* channelValues, unsigned universe, const Timing& timing)
	{
		double timeInMs = timing.BeatValue();
		unsigned step = (unsigned) fmod(timeInMs / _trigger.DelayInBeats(), _sequence.Size());
		_sequence.List()[step]->MixInput(0, value);
	}
	
	void mixSyncedChase(const ControlValue& value, unsigned* channelValues, unsigned universe, const Timing& timing)
	{
		unsigned step = (timing.TimestepNumber() / _trigger.DelayInSyncs()) % _sequence.Size();
		_sequence.List()[step]->MixInput(0, value);
	}
	
	void mixDelayChase(const ControlValue& value, unsigned* channelValues, unsigned universe, const Timing& timing)
	{
		double timeInMs = timing.TimeInMS();
		double chaseTime = fmod(timeInMs, _trigger.DelayInMs() + _transition.LengthInMs());
		unsigned step = (unsigned) fmod(timeInMs / (_trigger.DelayInMs() + _transition.LengthInMs()), _sequence.Size());
		if(chaseTime < _trigger.DelayInMs())
		{
			// We are not in a transition, just mix the corresponding controllable
			_sequence.List()[step]->MixInput(0, value);
		}
		else
		{
			// We are in a transition
			double transitionTime = chaseTime - _trigger.DelayInMs();
			Controllable
				&first = *_sequence.List()[step],
				&second = *_sequence.List()[(step + 1) % _sequence.Size()];
			_transition.Mix(first, 0, second, 0, transitionTime, value, timing);
		}
	}
	
	ControlValue _inputValue;
	class Sequence _sequence;
	class Trigger _trigger;
	class Transition _transition;
};

#endif
