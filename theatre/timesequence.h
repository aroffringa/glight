#ifndef THEATRE_TIME_SEQUENCE_H_
#define THEATRE_TIME_SEQUENCE_H_

#include <array>

#include "controllable.h"
#include "controlvalue.h"
#include "sequence.h"
#include "timing.h"
#include "transition.h"
#include "trigger.h"

namespace glight::theatre {

class TimeSequence final : public Controllable {
 public:
  TimeSequence()
      : _inputValue(),
        _activeValue{0, 0},
        _stepStart(),
        _stepNumber{0, 0},
        _transitionTriggered{false, false},
        _sequence(),
        _steps(),
        _sustain(false),
        _repeatCount(1) {}

  std::unique_ptr<TimeSequence> CopyWithoutSequence() const {
    return std::unique_ptr<TimeSequence>(new TimeSequence(*this));
  }

  size_t NInputs() const override { return 1; }

  ControlValue &InputValue(size_t) override { return _inputValue; }

  virtual FunctionType InputType(size_t) const override {
    return FunctionType::Master;
  }

  size_t NOutputs() const override { return _sequence.List().size(); }

  std::pair<const Controllable *, size_t> Output(size_t index) const override {
    return std::make_pair(_sequence.List()[index].GetControllable(),
                          _sequence.List()[index].InputIndex());
  }

  size_t RepeatCount() const { return _repeatCount; }
  /**
   * A repeat count of zero means indefinite.
   */
  void SetRepeatCount(size_t count) { _repeatCount = count; }

  bool Sustain() const { return _sustain; }
  void SetSustain(bool sustain) { _sustain = sustain; }

  virtual void Mix(const Timing &timing, bool primary) override {
    ControlValue &activeValue = _activeValue[primary];
    Timing &stepStart = _stepStart[primary];
    size_t &stepNumber = _stepNumber[primary];
    bool &transitionTriggered = _transitionTriggered[primary];
    if (_inputValue || (_sustain && activeValue)) {
      if (!activeValue) {
        // Start the sequence
        stepNumber = 0;
        stepStart = timing;
        transitionTriggered = false;
      }
      if (_repeatCount == 0 || stepNumber < _repeatCount * _steps.size()) {
        if (_sustain)
          activeValue = std::max(activeValue.UInt(), _inputValue.UInt());
        else
          activeValue = _inputValue;
        const Step &activeStep = _steps[stepNumber % _steps.size()];
        if (!transitionTriggered) {
          switch (activeStep.trigger.Type()) {
            case TriggerType::Delay: {
              const double timePassed =
                  timing.TimeInMS() - stepStart.TimeInMS();
              if (timePassed >= activeStep.trigger.DelayInMs()) {
                transitionTriggered = true;
                stepStart = timing;
              }
            } break;
            case TriggerType::Sync: {
              const size_t syncsPassed =
                  timing.TimestepNumber() - stepStart.TimestepNumber();
              if (syncsPassed >= activeStep.trigger.DelayInSyncs()) {
                transitionTriggered = true;
                stepStart = timing;
              }
            } break;
            case TriggerType::Beat: {
              const size_t beatsPassed =
                  timing.BeatValue() - stepStart.BeatValue();
              if (beatsPassed >= activeStep.trigger.DelayInBeats()) {
                transitionTriggered = true;
                stepStart = timing;
              }
            } break;
          }
          if (!transitionTriggered) {
            Input &input = _sequence.List()[stepNumber % _steps.size()];
            input.GetControllable()->MixInput(input.InputIndex(), activeValue);
          }
        }
        if (transitionTriggered) {
          // Are we in the final step?
          if (_repeatCount != 0 &&
              stepNumber + 1 >= _repeatCount * _steps.size()) {
            ++stepNumber;
            activeValue = _inputValue;
          } else {
            // Not there yet; transition to next state
            double transitionTime = timing.TimeInMS() - stepStart.TimeInMS();
            Input &a = _sequence.List()[stepNumber % _steps.size()];
            Input &b = _sequence.List()[(stepNumber + 1) % _steps.size()];
            if (transitionTime >= activeStep.transition.LengthInMs()) {
              ++stepNumber;
              stepStart = timing;
              transitionTriggered = false;
              b.GetControllable()->MixInput(b.InputIndex(), activeValue);
            } else {
              activeStep.transition.Mix(*a.GetControllable(), a.InputIndex(),
                                        *b.GetControllable(), b.InputIndex(),
                                        transitionTime, activeValue, timing);
            }
          }
        }
      } else {
        activeValue = _inputValue;
      }
    } else {
      activeValue = 0;
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
  std::array<ControlValue, 2> _activeValue;
  std::array<Timing, 2> _stepStart;
  std::array<size_t, 2> _stepNumber;
  std::array<bool, 2> _transitionTriggered;

  class Sequence _sequence;
  std::vector<Step> _steps;
  bool _sustain;
  size_t _repeatCount;
};

}  // namespace glight::theatre

#endif
