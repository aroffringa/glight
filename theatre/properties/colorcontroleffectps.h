#ifndef COLOR_CONTROL_EFFECT_PS_H_
#define COLOR_CONTROL_EFFECT_PS_H_

#include "propertyset.h"

#include "../../theatre/effects/colorcontroleffect.h"

namespace glight::theatre {

class ColorControlEffectPS final : public PropertySet {
 public:
  ColorControlEffectPS() {  }

 protected:
  void setControlValue(FolderObject &object, size_t index,
                               unsigned value) const override {
  }

  unsigned getControlValue(const FolderObject &object,
                                   size_t index) const override {  return 0; }
};

}  // namespace glight::theatre

#endif

