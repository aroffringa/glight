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
		Chase(class Sequence &sequence) : _sequence(sequence)
		{
		}
		Chase(const Chase& chase, class Sequence &sequence) :
			Controllable(chase),
			_sequence(sequence),
			_trigger(chase._trigger),
			_transition(chase._transition)
		{
		}
		~Chase()
		{
		}
		virtual void Mix(const ControlValue &value, unsigned *channelValues, unsigned universe, const Timing& timing)
		{
			if(_trigger.Type() == Trigger::DelayTriggered)
				mixDelayChase(value, channelValues, universe, timing);
			else
				mixBeatChase(value, channelValues, universe, timing);
		}
		
		const class Transition &Transition() const { return _transition; }
		class Transition &Transition() { return _transition; }

		const class Trigger &Trigger() const { return _trigger; }
		class Trigger &Trigger() { return _trigger; }

		class Sequence &Sequence() const { return _sequence; }

	private:
		void mixBeatChase(const ControlValue &value, unsigned *channelValues, unsigned universe, const Timing& timing)
		{
			double timeInMs = timing.BeatValue();
			unsigned step = (unsigned) fmod(timeInMs / _trigger.DelayInBeats(), _sequence.Size());
			_sequence.Presets()[step]->Mix(value, channelValues, universe, timing);
		}
		
		void mixDelayChase(const ControlValue &value, unsigned *channelValues, unsigned universe, const Timing& timing)
		{
			double timeInMs = timing.TimeInMS();
			double chaseTime = fmod(timeInMs, _trigger.DelayInMs() + _transition.LengthInMs());
			if(chaseTime < _trigger.DelayInMs())
			{
				// We are not in a transition, just mix the corresponding preset
				unsigned step = (unsigned) fmod(timeInMs / (_trigger.DelayInMs() + _transition.LengthInMs()), _sequence.Size());
				_sequence.Presets()[step]->Mix(value, channelValues, universe, timing);
			}
			else
			{
				// We are in a transition
				unsigned step = (unsigned) fmod(timeInMs / (_trigger.DelayInMs() + _transition.LengthInMs()), _sequence.Size());
				double transitionTime = chaseTime - _trigger.DelayInMs();
				PresetCollection
					&first = *_sequence.Presets()[step],
					&second = *_sequence.Presets()[(step + 1) % _sequence.Size()];
				_transition.Mix(first, second, transitionTime, value, channelValues, universe, timing);
			}
		}
		
		class Sequence &_sequence;
		class Trigger _trigger;
		class Transition _transition;
};

#endif
