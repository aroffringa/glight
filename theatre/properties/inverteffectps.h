#ifndef INVERT_EFFECT_PS
#define INVERT_EFFECT_PS

#include "propertyset.h"

#include "../effects/inverteffect.h"

class InvertEffectPS final : public PropertySet {
 public:
  InvertEffectPS() {
    addProperty(
        Property("offthreshold", "Off threshold", Property::ControlValue));
  }

 protected:
  virtual void setControlValue(FolderObject &object, size_t index,
                               unsigned value) const final override {
    InvertEffect &fx = static_cast<InvertEffect &>(object);
    switch (index) {
      case 0:
        fx.SetOffThreshold(value);
        break;
    }
  }

  virtual unsigned getControlValue(const FolderObject &object,
                                   size_t index) const final override {
    const InvertEffect &fx = static_cast<const InvertEffect &>(object);
    switch (index) {
      case 0:
        return fx.OffThreshold();
    }
    return 0;
  }
};

#endif
