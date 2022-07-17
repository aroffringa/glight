#ifndef AUDIO_LEVEL_EFFECT_PS
#define AUDIO_LEVEL_EFFECT_PS

#include "propertyset.h"

#include "../../theatre/effects/audioleveleffect.h"

namespace glight::theatre {

class AudioLevelEffectPS final : public PropertySet {
 public:
  AudioLevelEffectPS() {
    addProperty(Property("decayspeed", "Decay speed", Property::ControlValue));
  }

 protected:
  virtual void setControlValue(FolderObject &object, size_t index,
                               unsigned value) const final override {
    AudioLevelEffect &tfx = static_cast<AudioLevelEffect &>(object);
    switch (index) {
      case 0:
        tfx.SetDecaySpeed(value);
        break;
    }
  }

  virtual unsigned getControlValue(const FolderObject &object,
                                   size_t index) const final override {
    const AudioLevelEffect &tfx = static_cast<const AudioLevelEffect &>(object);
    switch (index) {
      case 0:
        return tfx.DecaySpeed();
    }
    return 0;
  }
};

}  // namespace glight::theatre

#endif
