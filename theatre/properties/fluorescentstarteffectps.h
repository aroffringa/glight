#ifndef FLUORESSCENT_START_EFFECT_PS
#define FLUORESSCENT_START_EFFECT_PS

#include "propertyset.h"

#include "../effects/fluorescentstarteffect.h"

class FluorescentStartEffectPS final : public PropertySet {
 public:
  FluorescentStartEffectPS() {
    addProperty(
        Property("averageduration", "Average duration", Property::Duration));
    addProperty(Property("stddev", "Standard deviation", Property::Duration));
    addProperty(
        Property("flashduration", "Flash duration", Property::Duration));
    addProperty(Property("glowvalue", "Glow value", Property::ControlValue));
    addProperty(Property("independentoutputs", "Independent outputs",
                         Property::Boolean));
  }

 protected:
  virtual void setDuration(FolderObject &object, size_t index,
                           double value) const final override {
    FluorescentStartEffect &fx = static_cast<FluorescentStartEffect &>(object);
    switch (index) {
      case 0:
        fx.SetAverageDuration(value);
        break;
      case 1:
        fx.SetStdDeviation(value);
        break;
      case 2:
        fx.SetFlashDuration(value);
        break;
    }
  }

  virtual double getDuration(const FolderObject &object,
                             size_t index) const final override {
    const FluorescentStartEffect &fx =
        static_cast<const FluorescentStartEffect &>(object);
    switch (index) {
      case 0:
        return fx.AverageDuration();
        break;
      case 1:
        return fx.StdDeviation();
        break;
      case 2:
        return fx.FlashDuration();
        break;
    }
    return 0;
  }

  virtual void setControlValue(FolderObject &object, size_t index,
                               unsigned value) const final override {
    FluorescentStartEffect &fx = static_cast<FluorescentStartEffect &>(object);
    switch (index) {
      case 3:
        fx.SetGlowValue(value);
        break;
    }
  }

  virtual unsigned getControlValue(const FolderObject &object,
                                   size_t index) const final override {
    const FluorescentStartEffect &fx =
        static_cast<const FluorescentStartEffect &>(object);
    switch (index) {
      case 3:
        return fx.GlowValue();
    }
    return 0;
  }

  virtual void setBool(FolderObject &object, size_t index,
                       bool value) const override {
    FluorescentStartEffect &fx = static_cast<FluorescentStartEffect &>(object);
    switch (index) {
      case 4:
        fx.SetIndependentOutputs(value);
        break;
    }
  }

  virtual bool getBool(const FolderObject &object,
                       size_t index) const final override {
    const FluorescentStartEffect &fx =
        static_cast<const FluorescentStartEffect &>(object);
    switch (index) {
      case 4:
        return fx.IndependentOutputs();
    }
    return false;
  }
};

#endif
