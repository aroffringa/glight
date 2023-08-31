#ifndef TWINKLE_EFFECT_PS_
#define TWINKLE_EFFECT_PS_

#include "propertyset.h"

#include "../effects/twinkleeffect.h"

namespace glight::theatre {

class TwinkleEffectPS final : public PropertySet {
 public:
  TwinkleEffectPS() {
    addProperty(
        Property("average_delay", "Average delay", PropertyType::Duration));
    addProperty(Property("hold_time", "Hold time", PropertyType::Duration));
    addProperty(
        Property("transition_in", "Transition in", PropertyType::Transition));
    addProperty(
        Property("transition_out", "Transition out", PropertyType::Transition));
  }

 protected:
  void setDuration(FolderObject &object, size_t index,
                   double value) const override {
    TwinkleEffect &tfx = static_cast<TwinkleEffect &>(object);
    switch (index) {
      case 0:
        tfx.SetAverageDelay(value);
        break;
      case 1:
        tfx.SetHoldTime(value);
        break;
    }
  }

  double getDuration(const FolderObject &object, size_t index) const override {
    const TwinkleEffect &tfx = static_cast<const TwinkleEffect &>(object);
    switch (index) {
      case 0:
        return tfx.AverageDelay();
      case 1:
        return tfx.HoldTime();
    }
    return 0;
  }

  void setTransition(FolderObject &object, size_t index,
                     const Transition &value) const override {
    TwinkleEffect &tfx = static_cast<TwinkleEffect &>(object);
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
                           size_t index) const override {
    const TwinkleEffect &tfx = static_cast<const TwinkleEffect &>(object);
    if (index == 2)
      return tfx.GetTransitionIn();
    else  // 3
      return tfx.GetTransitionOut();
  }
};

}  // namespace glight::theatre

#endif
