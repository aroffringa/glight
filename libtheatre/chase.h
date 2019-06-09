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
	
	std::unique_ptr<Chase> CopyWithoutSequence() const
	{
		return std::unique_ptr<Chase>(new Chase(*this));
	}

	size_t NInputs() const final override
	{ return 1; }
	
	ControlValue& InputValue(size_t) final override
	{ return _inputValue; }
	
	size_t NOutputs() const final override
	{ return _sequence.List().size(); }
	
	std::pair<Controllable*, size_t> Output(size_t index) const final override
	{ return _sequence.List()[index]; }
	
	virtual void Mix(unsigned *channelValues, unsigned universe, const Timing& timing) final override
	{
		switch(_trigger.Type()) {
		case Trigger::DelayTriggered:
			mixDelayChase(channelValues, universe, timing);
			break;
		case Trigger::SyncTriggered:
			mixSyncedChase(channelValues, universe, timing);
			break;
		case Trigger::BeatTriggered:
			mixBeatChase(channelValues, universe, timing);
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
	/**
	 * Copy constructor for dry copy
	 */
	Chase(const Chase& chase) :
		Controllable(chase),
		_trigger(chase._trigger),
		_transition(chase._transition)
	{ }
	
	void mixBeatChase(unsigned* channelValues, unsigned universe, const Timing& timing)
	{
		double timeInMs = timing.BeatValue();
		unsigned step = (unsigned) fmod(timeInMs / _trigger.DelayInBeats(), _sequence.Size());
		_sequence.List()[step].first->MixInput(_sequence.List()[step].second, _inputValue);
	}
	
	void mixSyncedChase(unsigned* channelValues, unsigned universe, const Timing& timing)
	{
		unsigned step = (timing.TimestepNumber() / _trigger.DelayInSyncs()) % _sequence.Size();
		_sequence.List()[step].first->MixInput(_sequence.List()[step].second, _inputValue);
	}
	
	void mixDelayChase(unsigned* channelValues, unsigned universe, const Timing& timing)
	{
		double timeInMs = timing.TimeInMS();
		double chaseTime = fmod(timeInMs, _trigger.DelayInMs() + _transition.LengthInMs());
		unsigned step = (unsigned) fmod(timeInMs / (_trigger.DelayInMs() + _transition.LengthInMs()), _sequence.Size());
		if(chaseTime < _trigger.DelayInMs())
		{
			// We are not in a transition, just mix the corresponding controllable
			_sequence.List()[step].first->MixInput(_sequence.List()[step].second, _inputValue);
		}
		else
		{
			// We are in a transition
			double transitionTime = chaseTime - _trigger.DelayInMs();
			Controllable
				&first = *_sequence.List()[step].first,
				&second = *_sequence.List()[(step + 1) % _sequence.Size()].first;
			_transition.Mix(first, _sequence.List()[step].second, second, _sequence.List()[(step + 1) % _sequence.Size()].second, transitionTime, _inputValue, timing);
		}
	}
	
	ControlValue _inputValue;
	class Sequence _sequence;
	class Trigger _trigger;
	class Transition _transition;
};

#endif
