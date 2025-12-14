#ifndef GLIGHT_THEATRE_TIMER_EFFECT_H_
#define GLIGHT_THEATRE_TIMER_EFFECT_H_

#include "../effect.h"
#include "../transition.h"

#include "system/optionalnumber.h"
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
        MixOn(values[0], timing, primary);
      } else {
        MixOff(values[0], timing, primary);
      }
    }
  }

 private:
  void MixOn(const ControlValue& input, const Timing& timing, bool primary) {
    if (!is_on_[primary]) {
      is_on_[primary] = true;
      transition_start_[primary] = timing.TimeInMS();
    }
    if (transition_start_[primary]) {
      const int start = *transition_start_[primary];
      if (timing.TimeInMS() - start < transition_in_.LengthInMs()) {
        const ControlValue multiplier =
            transition_in_.InValue(timing.TimeInMS() - start, timing);
        setAllOutputs(input * multiplier);
      } else {
        transition_start_[primary].Reset();
      }
    }
    if (!transition_start_[primary]) {
      setAllOutputs(input);
    }
  }

  void MixOff(const ControlValue& input, const Timing& timing, bool primary) {
    if (is_on_[primary]) {
      is_on_[primary] = false;
      transition_start_[primary] = timing.TimeInMS();
    }
    if (transition_start_[primary]) {
      const int start = *transition_start_[primary];
      if (timing.TimeInMS() - start < transition_out_.LengthInMs()) {
        const ControlValue multiplier =
            transition_out_.OutValue(timing.TimeInMS() - start, timing);
        setAllOutputs(input * multiplier);
      } else {
        transition_start_[primary].Reset();
      }
    }
  }

  Transition transition_in_ = Transition(300, TransitionType::Fade);
  Transition transition_out_ = Transition(300, TransitionType::Fade);
  system::TimePattern start_;
  system::TimePattern end_;
  /// If unset, no transition is ongoing. If set, it is the transition start
  /// time.
  system::OptionalNumber<int> transition_start_[2];
  bool is_on_[2] = {false, false};
};

}  // namespace glight::theatre

#endif
