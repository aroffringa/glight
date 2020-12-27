#ifndef FLICKER_EFFECT_PS
#define FLICKER_EFFECT_PS

#include "propertyset.h"

#include "../effects/flickereffect.h"

class FlickerEffectPS final : public PropertySet {
public:
  FlickerEffectPS() {
    addProperty(Property("speed", "Speed", Property::ControlValue));
    addProperty(Property("independentoutputs", "Independent outputs",
                         Property::Boolean));
  }

protected:
  virtual void setControlValue(FolderObject &object, size_t index,
                               unsigned value) const final override {
    FlickerEffect &fx = static_cast<FlickerEffect &>(object);
    switch (index) {
    case 0:
      fx.SetSpeed(value);
      break;
    }
  }

  virtual unsigned getControlValue(const FolderObject &object,
                                   size_t index) const final override {
    const FlickerEffect &fx = static_cast<const FlickerEffect &>(object);
    switch (index) {
    case 0:
      return fx.Speed();
    }
    return 0;
  }

  virtual void setBool(FolderObject &object, size_t index,
                       bool value) const override {
    FlickerEffect &fx = static_cast<FlickerEffect &>(object);
    switch (index) {
    case 1:
      fx.SetIndependentOutputs(value);
      break;
    }
  }

  virtual bool getBool(const FolderObject &object,
                       size_t index) const final override {
    const FlickerEffect &fx = static_cast<const FlickerEffect &>(object);
    switch (index) {
    case 1:
      return fx.IndependentOutputs();
    }
    return 0;
  }
};

#endif
