#ifndef TIME_SEQUENCE_H
#define TIME_SEQUENCE_H

#include "controllable.h"
#include "controlvalue.h"
#include "sequence.h"
#include "timing.h"
#include "trigger.h"
#include "transition.h"

/**
	@author Andre Offringa
*/
class TimeSequence : public Controllable {
public:
	TimeSequence() :
		_inputValue(),
		_lastValue(0),
		_stepStart(),
		_stepNumber(0),
		_transitionTriggered(0),
		_sequence(),
		_steps(),
		_repeatCount(1)
	{ }
	
	std::unique_ptr<TimeSequence> CopyWithoutSequence() const
	{
		return std::unique_ptr<TimeSequence>(new TimeSequence(*this));
	}

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
		if(_inputValue)
		{
			if(!_lastValue)
			{
				// Start the sequence
				_stepNumber = 0;
				_stepStart = timing;
				_transitionTriggered = false;
			}
			if(_repeatCount == 0 || _stepNumber < _repeatCount * _steps.size())
			{
				const Step& activeStep = _steps[_stepNumber % _steps.size()];
				if(!_transitionTriggered)
				{
					switch(activeStep.trigger.Type())
					{
						case Trigger::DelayTriggered:
						{
							double timePassed = timing.TimeInMS() - _stepStart.TimeInMS();
							if(timePassed >= activeStep.trigger.DelayInMs())
							{
								_transitionTriggered = true;
								_stepStart = timing;
							}
							else
								_sequence.List()[_stepNumber % _steps.size()]->MixInput(0, _inputValue);
						} break;
						case Trigger::SyncTriggered:
						{
							size_t syncsPassed = timing.TimestepNumber() - _stepStart.TimestepNumber();
							if(syncsPassed >= activeStep.trigger.DelayInSyncs())
							{
								_transitionTriggered = true;
								_stepStart = timing;
							}
						} break;
						case Trigger::BeatTriggered:
						{
							size_t beatsPassed = timing.BeatValue() - _stepStart.BeatValue();
							if(beatsPassed >= activeStep.trigger.DelayInBeats())
							{
								_transitionTriggered = true;
								_stepStart = timing;
							}
						} break;
					}
				}
				if(_transitionTriggered)
				{
					double transitionTime = timing.TimeInMS() - _stepStart.TimeInMS();
					Controllable
						&first = *_sequence.List()[_stepNumber % _steps.size()],
						&second = *_sequence.List()[(_stepNumber+1) % _steps.size()];
					activeStep.transition.Mix(first, 0, second, 0, transitionTime, _inputValue, timing);
					if(activeStep.transition.LengthInMs() > transitionTime)
					{
						++_stepNumber;
						_stepStart = timing;
						_transitionTriggered = false;
					}
				}
			}
		}
		_lastValue = _inputValue;
	}
	
	const class Sequence& Sequence() const { return _sequence; }
	class Sequence& Sequence() { return _sequence; }

	struct Step {
		Transition transition;
		Trigger trigger;
	};
private:
	/**
	 * Copy constructor for dry copy
	 */
	TimeSequence(const TimeSequence& timeSequence) :
		Controllable(timeSequence),
		_steps(timeSequence._steps)
	{ }
	
	ControlValue _inputValue;
	unsigned _lastValue;
	Timing _stepStart;
	size_t _stepNumber;
	bool _transitionTriggered;
	
	class Sequence _sequence;
	std::vector<Step> _steps;
	size_t _repeatCount;
};

#endif
