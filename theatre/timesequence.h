#ifndef TIME_SEQUENCE_H
#define TIME_SEQUENCE_H

#include "controllable.h"
#include "controlvalue.h"
#include "sequence.h"
#include "timing.h"
#include "transition.h"
#include "trigger.h"

/**
        @author Andre Offringa
*/
class TimeSequence : public Controllable {
 public:
  TimeSequence()
      : _inputValue(),
        _activeValue(0),
        _stepStart(),
        _stepNumber(0),
        _transitionTriggered(false),
        _sequence(),
        _steps(),
        _sustain(false),
        _repeatCount(1) {}

  std::unique_ptr<TimeSequence> CopyWithoutSequence() const {
    return std::unique_ptr<TimeSequence>(new TimeSequence(*this));
  }

  size_t NInputs() const final override { return 1; }

  ControlValue &InputValue(size_t) final override { return _inputValue; }

  virtual FunctionType InputType(size_t) const final override {
    return FunctionType::Master;
  }

  size_t NOutputs() const final override { return _sequence.List().size(); }

  std::pair<Controllable *, size_t> Output(size_t index) const final override {
    return std::make_pair(_sequence.List()[index].first,
                          _sequence.List()[index].second);
  }

  size_t RepeatCount() const { return _repeatCount; }
  /**
   * A repeat count of zero means indefinite.
   */
  void SetRepeatCount(size_t count) { _repeatCount = count; }

  bool Sustain() const { return _sustain; }
  void SetSustain(bool sustain) { _sustain = sustain; }

  virtual void Mix(const Timing &timing) final override {
    if (_inputValue || (_sustain && _activeValue)) {
      if (!_activeValue) {
        // Start the sequence
        _stepNumber = 0;
        _stepStart = timing;
        _transitionTriggered = false;
      }
      if (_repeatCount == 0 || _stepNumber < _repeatCount * _steps.size()) {
        if (_sustain)
          _activeValue = std::max(_activeValue.UInt(), _inputValue.UInt());
        else
          _activeValue = _inputValue;
        const Step &activeStep = _steps[_stepNumber % _steps.size()];
        if (!_transitionTriggered) {
          switch (activeStep.trigger.Type()) {
            case TriggerType::Delay: {
              double timePassed = timing.TimeInMS() - _stepStart.TimeInMS();
              if (timePassed >= activeStep.trigger.DelayInMs()) {
                _transitionTriggered = true;
                _stepStart = timing;
              }
            } break;
            case TriggerType::Sync: {
              size_t syncsPassed =
                  timing.TimestepNumber() - _stepStart.TimestepNumber();
              if (syncsPassed >= activeStep.trigger.DelayInSyncs()) {
                _transitionTriggered = true;
                _stepStart = timing;
              }
            } break;
            case TriggerType::Beat: {
              size_t beatsPassed = timing.BeatValue() - _stepStart.BeatValue();
              if (beatsPassed >= activeStep.trigger.DelayInBeats()) {
                _transitionTriggered = true;
                _stepStart = timing;
              }
            } break;
          }
          if (!_transitionTriggered) {
            const std::pair<Controllable *, size_t> &input =
                _sequence.List()[_stepNumber % _steps.size()];
            input.first->MixInput(input.second, _activeValue);
          }
        }
        if (_transitionTriggered) {
          // Are we in the final step?
          if (_repeatCount != 0 &&
              _stepNumber + 1 >= _repeatCount * _steps.size()) {
            ++_stepNumber;
            _activeValue = _inputValue;
          } else {
            // Not there yet; transition to next state
            double transitionTime = timing.TimeInMS() - _stepStart.TimeInMS();
            const std::pair<Controllable *, size_t>
                &a = _sequence.List()[_stepNumber % _steps.size()],
                &b = _sequence.List()[(_stepNumber + 1) % _steps.size()];
            if (transitionTime >= activeStep.transition.LengthInMs()) {
              ++_stepNumber;
              _stepStart = timing;
              _transitionTriggered = false;
              b.first->MixInput(b.second, _activeValue);
            } else {
              activeStep.transition.Mix(*a.first, a.second, *b.first, b.second,
                                        transitionTime, _activeValue, timing);
            }
          }
        }
      } else {
        _activeValue = _inputValue;
      }
    } else {
      _activeValue = 0;
    }
  }

  class Sequence &Sequence() {
    return _sequence;
  }
  const class Sequence &Sequence() const { return _sequence; }

  struct Step {
    Transition transition;
    Trigger trigger;
  };

  std::vector<Step> &Steps() { return _steps; }

  void AddStep(Controllable &controllable, size_t input) {
    _sequence.Add(controllable, input);
    _steps.emplace_back();
  }

  void RemoveStep(size_t index) {
    _sequence.Remove(index);
    _steps.erase(_steps.begin() + index);
  }

  const Step &GetStep(size_t index) const { return _steps[index]; }
  Step &GetStep(size_t index) { return _steps[index]; }

  size_t Size() const { return _steps.size(); }

 private:
  /**
   * Copy constructor for dry copy
   */
  TimeSequence(const TimeSequence &timeSequence)
      :

        Controllable(timeSequence),

        _inputValue(timeSequence._inputValue),
        _activeValue(timeSequence._activeValue),
        _stepStart(timeSequence._stepStart),
        _stepNumber(timeSequence._stepNumber),
        _transitionTriggered(timeSequence._transitionTriggered),

        _steps(timeSequence._steps),
        _sustain(timeSequence._sustain),
        _repeatCount(timeSequence._repeatCount) {}

  ControlValue _inputValue;
  ControlValue _activeValue;
  Timing _stepStart;
  size_t _stepNumber;
  bool _transitionTriggered;

  class Sequence _sequence;
  std::vector<Step> _steps;
  bool _sustain;
  size_t _repeatCount;
};

#endif
