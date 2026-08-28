#ifndef THEATRE_TIME_SEQUENCE_H_
#define THEATRE_TIME_SEQUENCE_H_

#include <array>

#include "controllable.h"
#include "controlvalue.h"
#include "input.h"
#include "timing.h"
#include "transition.h"
#include "trigger.h"

namespace glight::theatre {

class TimeSequence final : public Controllable {
 public:
  TimeSequence() = default;

  size_t NInputs() const override { return 1; }

  ControlValue &InputValue(size_t, bool primary) override { return _inputValue[primary]; }

  virtual FunctionType InputType(size_t) const override { return FunctionType::Master; }

  size_t NConnections() const override { return _sequence.size(); }

  std::pair<const Controllable *, size_t> GetConnection(size_t index) const override {
    return std::make_pair(_sequence[index].GetControllable(), _sequence[index].InputIndex());
  }

  size_t RepeatCount() const { return _repeatCount; }
  /**
   * A repeat count of zero means indefinite.
   */
  void SetRepeatCount(size_t count) { _repeatCount = count; }

  bool Sustain() const { return _sustain; }
  void SetSustain(bool sustain) { _sustain = sustain; }

  void Mix(const Timing &timing, bool primary) override {
    ControlValue &activeValue = _activeValue[primary];
    Timing &stepStart = _stepStart[primary];
    size_t &stepNumber = _stepNumber[primary];
    bool &transitionTriggered = _transitionTriggered[primary];
    if (_inputValue[primary] || (_sustain && activeValue)) {
      if (!activeValue) {
        // Start the sequence
        stepNumber = 0;
        stepStart = timing;
        transitionTriggered = false;
      }
      if (_repeatCount == 0 || stepNumber < _repeatCount * _steps.size()) {
        if (_sustain)
          activeValue = Max(activeValue, _inputValue[primary]);
        else
          activeValue = _inputValue[primary];
        const Step &activeStep = _steps[stepNumber % _steps.size()];
        if (!transitionTriggered) {
          switch (activeStep.trigger.Type()) {
            case TriggerType::Delay: {
              const double timePassed = timing.TimeInMS() - stepStart.TimeInMS();
              if (timePassed >= activeStep.trigger.DelayInMs()) {
                transitionTriggered = true;
                stepStart = timing;
              }
            } break;
            case TriggerType::Sync: {
              const size_t syncsPassed = timing.TimestepNumber() - stepStart.TimestepNumber();
              if (syncsPassed >= activeStep.trigger.DelayInSyncs()) {
                transitionTriggered = true;
                stepStart = timing;
              }
            } break;
            case TriggerType::Beat: {
              const size_t beatsPassed = timing.BeatValue() - stepStart.BeatValue();
              if (beatsPassed >= activeStep.trigger.DelayInBeats()) {
                transitionTriggered = true;
                stepStart = timing;
              }
            } break;
          }
          if (!transitionTriggered) {
            const size_t step_index = stepNumber % _steps.size();
            Input &input = _sequence[step_index];
            input.GetControllable()->MixInput(input.InputIndex(), activeValue,
                                              connection_values_[step_index][primary], primary);
            connection_values_[step_index][primary] = activeValue;
            MixInputsIf(_sequence, 0, connection_values_, primary,
                        [step_index](size_t i) { return i != step_index; });
          }
        }
        if (transitionTriggered) {
          // Are we in the final step?
          if (_repeatCount != 0 && stepNumber + 1 >= _repeatCount * _steps.size()) {
            ++stepNumber;
            activeValue = _inputValue[primary];
          } else {
            // Not there yet; transition to next state
            double transitionTime = timing.TimeInMS() - stepStart.TimeInMS();
            const size_t a_index = stepNumber % _steps.size();
            const size_t b_index = (stepNumber + 1) % _steps.size();
            Input &a = _sequence[a_index];
            Input &b = _sequence[b_index];
            if (transitionTime >= activeStep.transition.LengthInMs()) {
              ++stepNumber;
              stepStart = timing;
              transitionTriggered = false;
              b.GetControllable()->MixInput(b.InputIndex(), activeValue,
                                            connection_values_[b_index][primary], primary);
              MixInputsIf(_sequence, 0, connection_values_, primary,
                          [b_index](size_t i) { return i != b_index; });
            } else {
              const auto [first, second] =
                  activeStep.transition.Mix(transitionTime, activeValue, timing);
              a.GetControllable()->MixInput(a.InputIndex(), first,
                                            connection_values_[a_index][primary], primary);
              b.GetControllable()->MixInput(b.InputIndex(), second,
                                            connection_values_[b_index][primary], primary);
              connection_values_[a_index][primary] = first;
              connection_values_[b_index][primary] = second;
              MixInputsIf(_sequence, 0, connection_values_, primary,
                          [=](size_t i) { return i != a_index && i != b_index; });
            }
          }
        }
      } else {
        activeValue = _inputValue[primary];
      }
    } else {
      activeValue = ControlValue(0);
    }
  }

  const std::vector<Input> &Sequence() const { return _sequence; }

  template <typename VectorInput>
  void SetSequence(VectorInput &&sequence) {
    _sequence = std::forward<VectorInput>(sequence);
    connection_values_.assign(_sequence.size(), {ControlValue(), ControlValue()});
  }

  struct Step {
    Transition transition;
    Trigger trigger;
  };

  std::vector<Step> &Steps() { return _steps; }

  Step &AddStep(Controllable &controllable, size_t input) {
    _sequence.emplace_back(controllable, input);
    return _steps.emplace_back();
  }

  void RemoveStep(size_t index) {
    _sequence.erase(_sequence.begin() + index);
    _steps.erase(_steps.begin() + index);
  }

  const Step &GetStep(size_t index) const { return _steps[index]; }
  Step &GetStep(size_t index) { return _steps[index]; }

  size_t Size() const { return _steps.size(); }

 private:
  ControlValue _inputValue[2];
  std::array<ControlValue, 2> _activeValue;
  std::array<Timing, 2> _stepStart;
  std::array<size_t, 2> _stepNumber = {0, 0};
  std::array<bool, 2> _transitionTriggered = {false, false};

  std::vector<Input> _sequence;
  std::vector<std::array<ControlValue, 2>> connection_values_;
  std::vector<Step> _steps;
  bool _sustain = false;
  size_t _repeatCount = 1;
};

}  // namespace glight::theatre

#endif
