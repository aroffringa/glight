#ifndef THEATRE_CHASE_H_
#define THEATRE_CHASE_H_

#include <array>
#include <type_traits>
#include <vector>

#include "controllable.h"
#include "controlvalue.h"
#include "input.h"
#include "timing.h"
#include "transition.h"
#include "trigger.h"

namespace glight::theatre {

class Chase final : public Controllable {
 public:
  Chase() = default;

  size_t NInputs() const override { return 1; }

  ControlValue &InputValue(size_t) override { return input_value_; }

  virtual FunctionType InputType(size_t) const override { return FunctionType::Master; }

  size_t NConnections() const override { return sequence_.size(); }

  std::pair<const Controllable *, size_t> GetConnection(size_t index) const override {
    const Input &to_input = sequence_[index];
    return std::pair<const Controllable *, size_t>(to_input.GetControllable(),
                                                   to_input.InputIndex());
  }

  void Mix(const Timing &timing, bool primary) override {
    // Slowly drive the phase offset back to zero.
    if (phase_offset_ != 0.0) {
      if (phase_offset_ > 8.0)
        phase_offset_ -= 8.0;
      else if (phase_offset_ < -8.0)
        phase_offset_ += 8.0;
      else
        phase_offset_ = 0.0;
    }
    switch (trigger_.Type()) {
      case TriggerType::Delay:
        MixDelayChase(timing, primary);
        break;
      case TriggerType::Sync:
        MixSyncedChase(timing, primary);
        break;
      case TriggerType::Beat:
        MixBeatChase(timing, primary);
        break;
    }
  }

  const Transition &GetTransition() const { return transition_; }
  Transition &GetTransition() { return transition_; }

  const Trigger &GetTrigger() const { return trigger_; }
  Trigger &GetTrigger() { return trigger_; }

  const std::vector<Input> &GetSequence() const { return sequence_; }
  template <typename InputVector>
  void SetSequence(InputVector &&sequence) {
    sequence_ = std::forward<InputVector>(sequence);
    connection_values_.resize(sequence_.size());
  }

  void ShiftDelayTrigger(double triggerTime, double transitionTime, double currentTime) {
    double currentDuration = trigger_.DelayInMs() + transition_.LengthInMs();
    double currentPhase =
        std::fmod(currentTime + phase_offset_, currentDuration * sequence_.size());
    double stepPhase = std::fmod(currentPhase, currentDuration);
    unsigned step = (unsigned)fmod(currentPhase / currentDuration, sequence_.size());
    double newStepDuration = (triggerTime + transitionTime);
    double newDuration = newStepDuration * sequence_.size();
    if (stepPhase < trigger_.DelayInMs()) {
      // No transition is ongoing
      // Find an offset such that
      // (time + _phaseOffset) % duration = step*duration + stepPhase*old/new
      // phaseOffset = (step*stepDuration + stepPhase*old/new - time) % duration
      phase_offset_ = std::fmod(
          step * newStepDuration + stepPhase * triggerTime / trigger_.DelayInMs() - currentTime,
          newDuration);
    } else {
      // Transition ongoing: shift to the relative position inside the
      // transition Find an offset such that (time + _phaseOffset) % duration =
      // step*duration + stepPhase*old/new + trigger phaseOffset =
      // (step*stepDuration + transPhase*old/new + trigger - time) % duration
      phase_offset_ = std::fmod(
          step * newStepDuration +
              (stepPhase - trigger_.DelayInMs()) * transitionTime / transition_.LengthInMs() +
              triggerTime - currentTime,
          newDuration);
    }
    trigger_.SetDelayInMs(triggerTime);
    transition_.SetLengthInMs(transitionTime);
  }

  void ResetPhaseOffset() { phase_offset_ = 0.0; }

 private:
  void MixBeatChase(const Timing &timing, bool primary) {
    double timeInMs = timing.BeatValue();
    unsigned step = (unsigned)std::fmod(timeInMs / trigger_.DelayInBeats(), sequence_.size());
    sequence_[step].GetControllable()->MixInput(sequence_[step].InputIndex(), input_value_,
                                                connection_values_[step][primary]);
  }

  void MixSyncedChase(const Timing &timing, bool primary) {
    unsigned step = (timing.TimestepNumber() / trigger_.DelayInSyncs()) % sequence_.size();
    sequence_[step].GetControllable()->MixInput(sequence_[step].InputIndex(), input_value_,
                                                connection_values_[step][primary]);
  }

  void MixDelayChase(const Timing &timing, bool primary) {
    double timeInMs = timing.TimeInMS() + phase_offset_;
    double totalDuration = trigger_.DelayInMs() + transition_.LengthInMs();
    double phase = std::fmod(timeInMs, totalDuration);
    unsigned step = (unsigned)std::fmod(timeInMs / totalDuration, sequence_.size());
    if (phase < trigger_.DelayInMs()) {
      // We are not in a transition, just mix the corresponding controllable
      sequence_[step].GetControllable()->MixInput(sequence_[step].InputIndex(), input_value_,
                                                  connection_values_[step][primary]);
    } else {
      // We are in a transition
      const double transition_time = phase - trigger_.DelayInMs();
      Connection first = {.to_controllable = sequence_[step].GetControllable(),
                          .to_input_index = sequence_[step].InputIndex(),
                          .values = connection_values_[step]};
      const size_t next_step = (step + 1) % sequence_.size();
      Connection second = {.to_controllable = sequence_[next_step].GetControllable(),
                           .to_input_index = sequence_[next_step].InputIndex(),
                           .values = connection_values_[next_step]};
      transition_.Mix(first, second, transition_time, input_value_, timing, primary);
    }
  }

  ControlValue input_value_;
  std::vector<Input> sequence_;
  Trigger trigger_;
  Transition transition_;
  double phase_offset_ = 0.0;
  std::vector<std::array<ControlValue, 2>> connection_values_;
};

}  // namespace glight::theatre

#endif
