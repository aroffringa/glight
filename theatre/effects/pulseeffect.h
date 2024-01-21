#ifndef PULSE_EFFECT_H
#define PULSE_EFFECT_H

#include "../effect.h"
#include "../timing.h"
#include "../transition.h"

namespace glight::theatre {

class PulseEffect final : public Effect {
 public:
  PulseEffect() : Effect(1) {}

  virtual EffectType GetType() const override { return EffectType::Pulse; }

  Transition TransitionIn() const { return transition_in_; }
  void SetTransitionIn(const Transition& t_in) { transition_in_ = t_in; }

  unsigned Hold() const { return sustain_; }
  void SetHold(unsigned hold) { sustain_ = hold; }

  Transition TransitionOut() const { return transition_out_; }
  void SetTransitionOut(const Transition& t_out) { transition_out_ = t_out; }

  unsigned Sleep() const { return sleep_; }
  void SetSleep(unsigned sleep) { sleep_ = sleep; }

  bool Repeat() const { return _repeat; }
  void SetRepeat(bool repeat) { _repeat = repeat; }

 protected:
  virtual void MixImplementation(const ControlValue* values,
                                 const Timing& timing, bool primary) override {
    if (values[0].UInt() == 0) {
      is_active_[primary] = false;
    } else {
      if (!is_active_[primary]) {
        start_time_[primary] = timing.TimeInMS();
        is_active_[primary] = true;
      }
      double pos = timing.TimeInMS() - start_time_[primary];
      size_t cycle_duration = transition_in_.LengthInMs() + sustain_ +
                              transition_out_.LengthInMs() + sleep_;
      if (_repeat || pos < cycle_duration) {
        pos = std::fmod(pos, cycle_duration);
        bool handled = false;

        if (transition_in_.LengthInMs() != 0) {
          if (pos < transition_in_.LengthInMs()) {
            // Fade in
            const ControlValue value = transition_in_.InValue(pos, timing);
            setAllOutputs(values[0] * value);
            handled = true;
          } else {
            pos -= transition_in_.LengthInMs();
          }
        }

        if (sustain_ != 0 && !handled) {
          if (pos < sustain_) {
            setAllOutputs(ControlValue(values[0].UInt()));
            handled = true;
          } else
            pos -= sustain_;
        }

        if (transition_out_.LengthInMs() != 0 && !handled) {
          if (pos < transition_out_.LengthInMs()) {
            // Fade out
            const ControlValue value = transition_out_.OutValue(pos, timing);
            setAllOutputs(values[0] * value);
          }
        }
      }
    }
  }

 private:
  bool is_active_[2] = {false, false};
  double start_time_[2] = {0.0, 0.0};

  bool _repeat = false;
  Transition transition_in_;
  // In ms.
  unsigned sustain_ = 200;
  Transition transition_out_;
  // In ms.
  unsigned sleep_ = 200;
};

}  // namespace glight::theatre

#endif
