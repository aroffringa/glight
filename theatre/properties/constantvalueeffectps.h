#ifndef CONSTANT_VALUE_EFFECT_PS
#define CONSTANT_VALUE_EFFECT_PS

#include "propertyset.h"

#include "../effects/constantvalueeffect.h"

namespace glight::theatre {

class ConstantValueEffectPS final : public PropertySet {
 public:
  ConstantValueEffectPS() {
    addProperty(Property("value", "Value", Property::ControlValue));
  }

 protected:
  virtual void setControlValue(FolderObject &object, size_t index,
                               unsigned value) const final override {
    ConstantValueEffect &cfx = static_cast<ConstantValueEffect &>(object);
    switch (index) {
      case 0:
        cfx.SetValue(value);
        break;
    }
  }

  virtual unsigned getControlValue(const FolderObject &object,
                                   size_t index) const final override {
    const ConstantValueEffect &cfx =
        static_cast<const ConstantValueEffect &>(object);
    switch (index) {
      case 0:
        return cfx.Value();
    }
    return 0;
  }
};

}  // namespace glight::theatre

#endif
