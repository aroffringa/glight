#include "effect.h"

#include "effects/audioleveleffect.h"
#include "effects/constantvalueeffect.h"
#include "effects/curveeffect.h"
#include "effects/delayeffect.h"
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
    case ET::ConstantValue:
      return up(new ConstantValueEffect());
    case ET::Curve:
      return up(new CurveEffect());
    case ET::Delay:
      return up(new DelayEffect());
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

std::string Effect::TypeToName(EffectType type) {
  using ET = EffectType;
  switch (type) {
    case ET::AudioLevel:
      return "Audiolevel";
    case ET::ConstantValue:
      return "Constant value";
    case ET::Curve:
      return "Curve";
    case ET::Delay:
      return "Delay";
    case ET::Fade:
      return "Fade";
    case ET::Flicker:
      return "Flicker";
    case ET::FluorescentStart:
      return "Fluorescent start";
    case ET::Hub:
      return "Hub";
    case ET::Invert:
      return "Invert";
    case ET::MusicActivation:
      return "Music activation";
    case ET::Pulse:
      return "Pulse";
    case ET::RandomSelect:
      return "Random select";
    case ET::Threshold:
      return "Threshold";
  }
  return std::string();
}

EffectType Effect::NameToType(const std::string &name) {
  using ET = EffectType;
  if (name == "Audiolevel")
    return ET::AudioLevel;
  else if (name == "Constant value")
    return ET::ConstantValue;
  else if (name == "Curve")
    return ET::Curve;
  else if (name == "Delay")
    return ET::Delay;
  else if (name == "Fade")
    return ET::Fade;
  else if (name == "Flicker")
    return ET::Flicker;
  else if (name == "Fluorescent start")
    return ET::FluorescentStart;
  else if (name == "Invert")
    return ET::Invert;
  else if (name == "Music activation")
    return ET::MusicActivation;
  else if (name == "Pulse")
    return ET::Pulse;
  else if (name == "Random select")
    return ET::RandomSelect;
  else if (name == "Threshold")
    return ET::Threshold;
  else
    throw std::runtime_error("Unknown effect type");
}

std::vector<EffectType> Effect::GetTypes() {
  using ET = EffectType;
  return std::vector<ET>{ET::AudioLevel,
                         ET::ConstantValue,
                         ET::Curve,
                         ET::Delay,
                         ET::Fade,
                         ET::Flicker,
                         ET::FluorescentStart,
                         ET::Hub,
                         ET::Invert,
                         ET::MusicActivation,
                         ET::Pulse,
                         ET::RandomSelect,
                         ET::Threshold};
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
