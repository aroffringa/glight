#ifndef COLOR_TEMPERATURE_EFFECT_PS_H_
#define COLOR_TEMPERATURE_EFFECT_PS_H_

#include "propertyset.h"

#include "../../theatre/effects/colortemperatureeffect.h"

namespace glight::theatre {

class ColorTemperatureEffectPS final : public PropertySet {
 public:
  ColorTemperatureEffectPS() {
    addProperty(Property("min-temperature", "Minimum temperature",
                         PropertyType::Integer));
    addProperty(Property("max-temperature", "Maximum temperature",
                         PropertyType::Integer));
  }

 protected:
  virtual void setInteger(FolderObject &object, size_t index,
                          int value) const override {
    ColorTemperatureEffect &ctfx =
        static_cast<ColorTemperatureEffect &>(object);
    switch (index) {
      case 0:
        ctfx.SetMinimumTemperature(value);
        break;
      case 1:
        ctfx.SetMaximumTemperature(value);
        break;
    }
  }

  virtual int getInteger(const FolderObject &object,
                         size_t index) const override {
    const ColorTemperatureEffect &ctfx =
        static_cast<const ColorTemperatureEffect &>(object);
    switch (index) {
      case 0:
        return ctfx.MinimumTemperature();
      case 1:
        return ctfx.MaximumTemperature();
    }
    return 0;
  }
};

}  // namespace glight::theatre

#endif
