#ifndef TIMER_EFFECT_PS_
#define TIMER_EFFECT_PS_

#include "propertyset.h"

#include "theatre/effects/timereffect.h"

namespace glight::theatre {

class TimerEffectPS final : public PropertySet {
 public:
  TimerEffectPS() {
    addProperty(
        Property("start_pattern", "Start pattern", PropertyType::TimePattern));
    addProperty(
        Property("end_pattern", "End pattern", PropertyType::TimePattern));
    addProperty(
        Property("transition_in", "Transition in", PropertyType::Transition));
    addProperty(
        Property("transition_out", "Transition out", PropertyType::Transition));
  }

 protected:
  void setTimePattern(FolderObject &object, size_t index,
                      const system::TimePattern &value) const final {
    TimerEffect &tfx = static_cast<TimerEffect &>(object);
    switch (index) {
      case 0:
        tfx.SetStartPattern(value);
        break;
      case 1:
        tfx.SetEndPattern(value);
        break;
    }
  }

  const system::TimePattern &getTimePattern(const FolderObject &object,
                                            size_t index) const final {
    const TimerEffect &tfx = static_cast<const TimerEffect &>(object);
    switch (index) {
      case 0:
      default:
        return tfx.StartPattern();
      case 1:
        return tfx.EndPattern();
    }
  }

  void setTransition(FolderObject &object, size_t index,
                     const Transition &value) const final {
    TimerEffect &tfx = static_cast<TimerEffect &>(object);
    switch (index) {
      case 2:
        tfx.SetTransitionIn(value);
        break;
      case 3:
        tfx.SetTransitionOut(value);
        break;
    }
  }

  Transition getTransition(const FolderObject &object,
                           size_t index) const final {
    const TimerEffect &tfx = static_cast<const TimerEffect &>(object);
    if (index == 2)
      return tfx.GetTransitionIn();
    else  // 3
      return tfx.GetTransitionOut();
  }
};

}  // namespace glight::theatre

#endif
