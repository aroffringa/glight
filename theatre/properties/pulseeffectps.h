#ifndef THEATRE_PULSE_EFFECT_PS_
#define THEATRE_PULSE_EFFECT_PS_

#include "propertyset.h"

#include "../effects/pulseeffect.h"

namespace glight::theatre {

class PulseEffectPS final : public PropertySet {
 public:
  PulseEffectPS() {
    addProperty(Property("repeat", "Repeat", PropertyType::Boolean));
    addProperty(
        Property("transition_in", "Transition in", PropertyType::Transition));
    addProperty(Property("hold", "Hold", PropertyType::Duration));
    addProperty(
        Property("transition_out", "Transition out", PropertyType::Transition));
    addProperty(Property("sleep", "Sleep", PropertyType::Duration));
  }

 protected:
  void setDuration(FolderObject &object, size_t index,
                   double value) const final override {
    PulseEffect &pfx = static_cast<PulseEffect &>(object);
    switch (index) {
      case 2:
        pfx.SetHold(value);
        break;
      case 4:
        pfx.SetSleep(value);
        break;
    }
  }

  double getDuration(const FolderObject &object,
                     size_t index) const final override {
    const PulseEffect &pfx = static_cast<const PulseEffect &>(object);
    switch (index) {
      case 2:
        return pfx.Hold();
      case 4:
        return pfx.Sleep();
    }
    return 0;
  }

  void setBool(FolderObject &object, size_t, bool value) const override {
    PulseEffect &pfx = static_cast<PulseEffect &>(object);
    pfx.SetRepeat(value);
  }

  bool getBool(const FolderObject &object, size_t) const override {
    const PulseEffect &pfx = static_cast<const PulseEffect &>(object);
    return pfx.Repeat();
  }

  void setTransition(FolderObject &object, size_t index,
                     const Transition &value) const override {
    PulseEffect &pfx = static_cast<PulseEffect &>(object);
    if (index == 1)
      pfx.SetTransitionIn(value);
    else
      pfx.SetTransitionOut(value);
  }

  Transition getTransition(const FolderObject &object,
                           size_t index) const override {
    const PulseEffect &pfx = static_cast<const PulseEffect &>(object);
    if (index == 1)
      return pfx.TransitionIn();
    else  // 3
      return pfx.TransitionOut();
  }
};

}  // namespace glight::theatre

#endif
