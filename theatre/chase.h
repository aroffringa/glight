#ifndef THEATRE_CHASE_H_
#define THEATRE_CHASE_H_

#include "controllable.h"
#include "sequence.h"
#include "timing.h"
#include "transition.h"
#include "trigger.h"

namespace glight::theatre {

/**
        @author Andre Offringa
*/
class Chase final : public Controllable {
 public:
  Chase() : _phaseOffset(0.0) {}

  std::unique_ptr<Chase> CopyWithoutSequence() const {
    return std::unique_ptr<Chase>(new Chase(*this));
  }

  size_t NInputs() const override { return 1; }

  ControlValue &InputValue(size_t) override { return _inputValue; }

  virtual FunctionType InputType(size_t) const override {
    return FunctionType::Master;
  }

  size_t NOutputs() const override { return _sequence.List().size(); }

  std::pair<const Controllable *, size_t> Output(size_t index) const override {
    const Input &input = _sequence.List()[index];
    return std::pair<const Controllable *, size_t>(input.GetControllable(),
                                                   input.InputIndex());
  }

  virtual void Mix(const Timing &timing) override {
    // Slowly drive the phase offset back to zero.
    if (_phaseOffset != 0.0) {
      if (_phaseOffset > 8.0)
        _phaseOffset -= 8.0;
      else if (_phaseOffset < -8.0)
        _phaseOffset += 8.0;
      else
        _phaseOffset = 0.0;
    }
    switch (_trigger.Type()) {
      case TriggerType::Delay:
        mixDelayChase(timing);
        break;
      case TriggerType::Sync:
        mixSyncedChase(timing);
        break;
      case TriggerType::Beat:
        mixBeatChase(timing);
        break;
    }
  }

  const class Transition &Transition() const { return _transition; }
  class Transition &Transition() {
    return _transition;
  }

  const class Trigger &Trigger() const { return _trigger; }
  class Trigger &Trigger() {
    return _trigger;
  }

  const class Sequence &Sequence() const { return _sequence; }
  class Sequence &Sequence() {
    return _sequence;
  }

  void ShiftDelayTrigger(double triggerTime, double transitionTime,
                         double currentTime) {
    double currentDuration = _trigger.DelayInMs() + _transition.LengthInMs();
    double currentPhase = std::fmod(currentTime + _phaseOffset,
                                    currentDuration * _sequence.Size());
    double stepPhase = std::fmod(currentPhase, currentDuration);
    unsigned step =
        (unsigned)fmod(currentPhase / currentDuration, _sequence.Size());
    double newStepDuration = (triggerTime + transitionTime);
    double newDuration = newStepDuration * _sequence.Size();
    if (stepPhase < _trigger.DelayInMs()) {
      // No transition is ongoing
      // Find an offset such that
      // (time + _phaseOffset) % duration = step*duration + stepPhase*old/new
      // phaseOffset = (step*stepDuration + stepPhase*old/new - time) % duration
      _phaseOffset = std::fmod(
          step * newStepDuration +
              stepPhase * triggerTime / _trigger.DelayInMs() - currentTime,
          newDuration);
    } else {
      // Transition ongoing: shift to the relative position inside the
      // transition Find an offset such that (time + _phaseOffset) % duration =
      // step*duration + stepPhase*old/new + trigger phaseOffset =
      // (step*stepDuration + transPhase*old/new + trigger - time) % duration
      _phaseOffset =
          std::fmod(step * newStepDuration +
                        (stepPhase - _trigger.DelayInMs()) * transitionTime /
                            _transition.LengthInMs() +
                        triggerTime - currentTime,
                    newDuration);
    }
    _trigger.SetDelayInMs(triggerTime);
    _transition.SetLengthInMs(transitionTime);
  }

  void ResetPhaseOffset() { _phaseOffset = 0.0; }

 private:
  /**
   * Copy constructor for dry copy
   */
  Chase(const Chase &chase)
      : Controllable(chase),
        _trigger(chase._trigger),
        _transition(chase._transition),
        _phaseOffset(chase._phaseOffset) {}

  void mixBeatChase(const Timing &timing) {
    double timeInMs = timing.BeatValue();
    unsigned step =
        (unsigned)fmod(timeInMs / _trigger.DelayInBeats(), _sequence.Size());
    _sequence.List()[step].GetControllable()->MixInput(
        _sequence.List()[step].InputIndex(), _inputValue);
  }

  void mixSyncedChase(const Timing &timing) {
    unsigned step =
        (timing.TimestepNumber() / _trigger.DelayInSyncs()) % _sequence.Size();
    _sequence.List()[step].GetControllable()->MixInput(
        _sequence.List()[step].InputIndex(), _inputValue);
  }

  void mixDelayChase(const Timing &timing) {
    double timeInMs = timing.TimeInMS() + _phaseOffset;
    double totalDuration = _trigger.DelayInMs() + _transition.LengthInMs();
    double phase = fmod(timeInMs, totalDuration);
    unsigned step = (unsigned)fmod(timeInMs / totalDuration, _sequence.Size());
    if (phase < _trigger.DelayInMs()) {
      // We are not in a transition, just mix the corresponding controllable
      _sequence.List()[step].GetControllable()->MixInput(
          _sequence.List()[step].InputIndex(), _inputValue);
    } else {
      // We are in a transition
      double transitionTime = phase - _trigger.DelayInMs();
      Controllable &first = *_sequence.List()[step].GetControllable();
      Controllable &second =
          *_sequence.List()[(step + 1) % _sequence.Size()].GetControllable();
      _transition.Mix(
          first, _sequence.List()[step].InputIndex(), second,
          _sequence.List()[(step + 1) % _sequence.Size()].InputIndex(),
          transitionTime, _inputValue, timing);
    }
  }

  ControlValue _inputValue;
  class Sequence _sequence;
  class Trigger _trigger;
  class Transition _transition;
  double _phaseOffset;
};

}  // namespace glight::theatre

#endif
