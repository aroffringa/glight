#ifndef PULSE_EFFECT_PS
#define PULSE_EFFECT_PS

#include "propertyset.h"

#include "../effects/pulseeffect.h"

class PulseEffectPS final : public PropertySet {
 public:
  PulseEffectPS() {
    addProperty(Property("repeat", "Repeat", Property::Boolean));
    addProperty(Property("attack", "Attack", Property::Duration));
    addProperty(Property("hold", "Hold", Property::Duration));
    addProperty(Property("release", "Release", Property::Duration));
    addProperty(Property("sleep", "Sleep", Property::Duration));
  }

 protected:
  void setDuration(FolderObject &object, size_t index,
                   double value) const final override {
    PulseEffect &pfx = static_cast<PulseEffect &>(object);
    switch (index) {
      case 1:
        pfx.SetAttack(value);
        break;
      case 2:
        pfx.SetHold(value);
        break;
      case 3:
        pfx.SetRelease(value);
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
      case 1:
        return pfx.Attack();
      case 2:
        return pfx.Hold();
      case 3:
        return pfx.Release();
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
};

#endif
