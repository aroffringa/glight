#ifndef GLIGHT_THEATRE_TIMER_EFFECT_H_
#define GLIGHT_THEATRE_TIMER_EFFECT_H_

#include "../effect.h"
#include "../transition.h"

#include "system/timepattern.h"

namespace glight::theatre {

class TimerEffect final : public Effect {
 public:
  TimerEffect() : Effect(1) {}

  virtual EffectType GetType() const final { return EffectType::Timer; }

  void SetTransitionIn(const Transition& transition) {
    transition_in_ = transition;
  }
  const Transition& GetTransitionIn() const { return transition_in_; }

  void SetTransitionOut(const Transition& transition) {
    transition_out_ = transition;
  }
  const Transition& GetTransitionOut() const { return transition_out_; }

  const system::TimePattern& StartPattern() const { return start_; }
  void SetStartPattern(const system::TimePattern& start) { start_ = start; }

  const system::TimePattern& EndPattern() const { return end_; }
  void SetEndPattern(const system::TimePattern& end) { end_ = end; }

 protected:
  virtual void MixImplementation(const ControlValue* values,
                                 const Timing& timing, bool primary) final {
    if (values[0]) {
      if (NowInRange(start_, end_)) {
        if (!is_on_) {
          is_on_ = true;
          transition_start_ = timing.TimeInMS();
        }
        if (timing.TimeInMS() - transition_start_ >
            transition_in_.LengthInMs()) {
          setAllOutputs(values[0]);
        } else {
          ControlValue multiplier = transition_in_.InValue(
              timing.TimeInMS() - transition_start_, timing);
          setAllOutputs(values[0] * multiplier);
        }
      } else {
        if (is_on_) {
          is_on_ = false;
          transition_start_ = timing.TimeInMS();
        }
        if (timing.TimeInMS() - transition_start_ <=
            transition_out_.LengthInMs()) {
          ControlValue multiplier = transition_out_.OutValue(
              timing.TimeInMS() - transition_start_, timing);
          setAllOutputs(values[0] * multiplier);
        }
      }
    }
  }

 private:
  Transition transition_in_ = Transition(300, TransitionType::Fade);
  Transition transition_out_ = Transition(300, TransitionType::Fade);
  system::TimePattern start_;
  system::TimePattern end_;
  int transition_start_ = 0;
  bool is_on_ = false;
};

}  // namespace glight::theatre

#endif
