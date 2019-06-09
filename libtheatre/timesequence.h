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
		_transitionTriggered(false),
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
	{ return std::make_pair(_sequence.List()[index].first, _sequence.List()[index].second); }
	
	size_t RepeatCount() const { return _repeatCount; }
	void SetRepeatCount(size_t count) { _repeatCount = count; }
	
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
					if(!_transitionTriggered)
					{
						const std::pair<Controllable*, size_t>& input = _sequence.List()[_stepNumber % _steps.size()];
						input.first->MixInput(input.second, _inputValue);
					}
				}
				if(_transitionTriggered)
				{
					if(_repeatCount != 0 && _stepNumber+1 == _repeatCount * _steps.size())
					{
						++_stepNumber;
						_stepStart = timing;
						_transitionTriggered = false;
					}
					else {
						double transitionTime = timing.TimeInMS() - _stepStart.TimeInMS();
						const std::pair<Controllable*, size_t>
							&a = _sequence.List()[_stepNumber % _steps.size()],
							&b = _sequence.List()[(_stepNumber+1) % _steps.size()];
						if(transitionTime >= activeStep.transition.LengthInMs())
						{
							++_stepNumber;
							_stepStart = timing;
							_transitionTriggered = false;
							b.first->MixInput(b.second, _inputValue);
						}
						else {
							activeStep.transition.Mix(*a.first, a.second, *b.first, b.second, transitionTime, _inputValue, timing);
						}
					}
				}
			}
		}
		_lastValue = _inputValue;
	}
	
	const class Sequence& Sequence() const { return _sequence; }
	
	struct Step {
		Transition transition;
		Trigger trigger;
	};
	
	void AddStep(Controllable* controllable, size_t input)
	{
		_sequence.Add(controllable, input);
		_steps.emplace_back();
	}
	
	const Step& GetStep(size_t index) const { return _steps[index]; }
	Step& GetStep(size_t index) { return _steps[index]; }
	
	size_t Size() const { return _steps.size(); }
	
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
