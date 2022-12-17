#include "effect.h"

#include "effects/audioleveleffect.h"
#include "effects/colorcontroleffect.h"
#include "effects/colortemperatureeffect.h"
#include "effects/constantvalueeffect.h"
#include "effects/curveeffect.h"
#include "effects/delayeffect.h"
#include "effects/dispensereffect.h"
#include "effects/fadeeffect.h"
#include "effects/flickereffect.h"
#include "effects/fluorescentstarteffect.h"
#include "effects/inverteffect.h"
#include "effects/musicactivationeffect.h"
#include "effects/pulseeffect.h"
#include "effects/randomselecteffect.h"
#include "effects/thresholdeffect.h"

#include "properties/propertyset.h"

namespace glight::theatre {

std::unique_ptr<Effect> Effect::Make(EffectType type) {
  using up = std::unique_ptr<Effect>;
  using ET = EffectType;
  switch (type) {
    case ET::AudioLevel:
      return up(new AudioLevelEffect());
    case ET::ColorControl:
      return up(new ColorControlEffect());
    case ET::ColorTemperature:
      return up(new ColorTemperatureEffect());
    case ET::ConstantValue:
      return up(new ConstantValueEffect());
    case ET::Curve:
      return up(new CurveEffect());
    case ET::Delay:
      return up(new DelayEffect());
    case ET::Dispenser:
      return up(new DispenserEffect());
    case ET::Fade:
      return up(new FadeEffect());
    case ET::Flicker:
      return up(new FlickerEffect());
    case ET::FluorescentStart:
      return up(new FluorescentStartEffect());
    case ET::Invert:
      return up(new InvertEffect());
    case ET::MusicActivation:
      return up(new MusicActivationEffect());
    case ET::Pulse:
      return up(new PulseEffect());
    case ET::RandomSelect:
      return up(new RandomSelectEffect());
    case ET::Threshold:
      return up(new ThresholdEffect());
  }
  return nullptr;
}

std::unique_ptr<Effect> Effect::Copy() const {
  std::unique_ptr<Effect> copy = Make(GetType());
  std::unique_ptr<PropertySet> psSrc = PropertySet::Make(*this),
                               psDest = PropertySet::Make(*copy);
  for (size_t i = 0; i != psSrc->size(); ++i) {
    psDest->AssignProperty((*psDest)[i], (*psSrc)[i], *psSrc);
  }
  copy->SetName(Name());
  return copy;
}

}  // namespace glight::theatre
