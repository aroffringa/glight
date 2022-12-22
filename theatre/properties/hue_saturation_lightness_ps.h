#ifndef HSL_EFFECT_PS_H_
#define HSL_EFFECT_PS_H_

#include "propertyset.h"

#include "../../theatre/effects/hue_saturation_lightness_effect.h"

namespace glight::theatre {

class HueSaturationLightnessEffectPS final : public PropertySet {
 public:
  HueSaturationLightnessEffectPS() {}

 protected:
  void setControlValue(FolderObject &object, size_t index,
                       unsigned value) const override {}

  unsigned getControlValue(const FolderObject &object,
                           size_t index) const override {
    return 0;
  }
};

}  // namespace glight::theatre

#endif
