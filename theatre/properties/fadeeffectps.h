#ifndef FADE_EFFECT_PS
#define FADE_EFFECT_PS

#include "propertyset.h"

#include "../effects/fadeeffect.h"

namespace glight::theatre {

class FadeEffectPS final : public PropertySet {
 public:
  FadeEffectPS() {
    addProperty(
        Property("upduration", "Up fading duration", PropertyType::Duration));
    addProperty(Property("downduration", "Down fading duration",
                         PropertyType::Duration));
    addProperty(
        Property("sustaintime", "Sustain time", PropertyType::Duration));
  }

 protected:
  virtual void setDuration(FolderObject &object, size_t index,
                           double value) const final override {
    FadeEffect &fadefx = static_cast<FadeEffect &>(object);
    switch (index) {
      case 0:
        fadefx.SetFadeUpDuration(value);
        break;
      case 1:
        fadefx.SetFadeDownDuration(value);
        break;
      case 2:
        fadefx.SetSustain(value);
        break;
    }
  }

  virtual double getDuration(const FolderObject &object,
                             size_t index) const final override {
    const FadeEffect &fadefx = static_cast<const FadeEffect &>(object);
    switch (index) {
      case 0:
        return fadefx.FadeUpDuration();
      case 1:
        return fadefx.FadeDownDuration();
      case 2:
        return fadefx.Sustain();
    }
    return 0;
  }
};

}  // namespace glight::theatre

#endif
