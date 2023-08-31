#include "propertyset.h"

#include "audioleveleffectps.h"
#include "colorcontroleffectps.h"
#include "colortemperatureeffectps.h"
#include "constantvalueeffectps.h"
#include "curveeffectps.h"
#include "delayeffectps.h"
#include "dispensereffectps.h"
#include "fadeeffectps.h"
#include "flickereffectps.h"
#include "fluorescentstarteffectps.h"
#include "hue_saturation_lightness_ps.h"
#include "inverteffectps.h"
#include "musicactivationeffectps.h"
#include "pulseeffectps.h"
#include "randomselecteffectps.h"
#include "thresholdeffectps.h"
#include "twinkleeffectps.h"

namespace glight::theatre {

#define FXCASE(X)                         \
  case EffectType::X:                     \
    ps = std::make_unique<X##EffectPS>(); \
    break;

std::unique_ptr<PropertySet> PropertySet::Make(FolderObject &object) {
  const Effect *effect = dynamic_cast<const Effect *>(&object);
  if (!effect)
    throw std::runtime_error(
        "Non-effect object type specified in call to PropertySet::Make()");
  std::unique_ptr<PropertySet> ps;
  switch (effect->GetType()) {
    FXCASE(AudioLevel);
    FXCASE(ColorControl);
    FXCASE(ColorTemperature);
    FXCASE(ConstantValue);
    FXCASE(Curve);
    FXCASE(Delay);
    FXCASE(Dispenser);
    FXCASE(Fade);
    FXCASE(Flicker);
    FXCASE(FluorescentStart);
    FXCASE(HueSaturationLightness);
    FXCASE(Invert);
    FXCASE(MusicActivation);
    FXCASE(Pulse);
    FXCASE(RandomSelect);
    FXCASE(Threshold);
    FXCASE(Twinkle);
  }
  if (!ps)
    throw std::runtime_error(
        "Unknown effect type in call to PropertySet::Make()");
  ps->_object = &object;
  return ps;
}

void PropertySet::AssignProperty(const Property &to, const Property &from,
                                 const PropertySet &fromSet) {
  if (from._type != to._type)
    throw std::runtime_error("Copying different types");
  switch (from._type) {
    case PropertyType::Boolean:
      SetBool(to, fromSet.GetBool(from));
      break;
    case PropertyType::Choice:
      SetChoice(to, fromSet.GetChoice(from));
      break;
    case PropertyType::ControlValue:
      SetControlValue(to, fromSet.GetControlValue(from));
      break;
    case PropertyType::Duration:
      SetDuration(to, fromSet.GetDuration(from));
      break;
    case PropertyType::Integer:
      SetInteger(to, fromSet.GetInteger(from));
      break;
    case PropertyType::Transition:
      SetTransition(to, fromSet.GetTransition(from));
      break;
  }
}

}  // namespace glight::theatre
