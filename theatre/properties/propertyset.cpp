#include "propertyset.h"

#include <cassert>

#include "audioleveleffectps.h"
#include "colorcontroleffectps.h"
#include "colortemperatureeffectps.h"
#include "constantvalueeffectps.h"
#include "curveeffectps.h"
#include "delayeffectps.h"
#include "fadeeffectps.h"
#include "flickereffectps.h"
#include "fluorescentstarteffectps.h"
#include "functiongeneratorps.h"
#include "hue_saturation_lightness_ps.h"
#include "inverteffectps.h"
#include "musicactivationeffectps.h"
#include "pulseeffectps.h"
#include "randomselecteffectps.h"
#include "thresholdeffectps.h"
#include "timereffectps.h"
#include "twinkleeffectps.h"

namespace glight::theatre {

#define FXCASE(X)                         \
  case EffectType::X:                     \
    ps = std::make_unique<X##EffectPS>(); \
    break;

#define EMPTYPSCASE(X)                \
  case EffectType::X:                 \
    ps = std::make_unique<EmptyPS>(); \
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
    EMPTYPSCASE(Dispenser);
    FXCASE(Fade);
    FXCASE(Flicker);
    FXCASE(FluorescentStart);
    FXCASE(FunctionGenerator);
    FXCASE(HueSaturationLightness);
    FXCASE(Invert);
    FXCASE(MusicActivation);
    FXCASE(Pulse);
    FXCASE(RandomSelect);
    EMPTYPSCASE(RgbMaster);
    FXCASE(Threshold);
    FXCASE(Timer);
    FXCASE(Twinkle);
    EMPTYPSCASE(Variable);
  }
  if (!ps)
    throw std::runtime_error(
        "Unknown effect type in call to PropertySet::Make()");
  ps->_object = &object;
  return ps;
}

void PropertySet::AssignProperty(const Property &to, const Property &from,
                                 const PropertySet &fromSet) {
  assert(from.type_ == to.type_);
  switch (from.type_) {
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
    case PropertyType::TimePattern:
      SetTimePattern(to, fromSet.GetTimePattern(from));
      break;
    case PropertyType::Transition:
      SetTransition(to, fromSet.GetTransition(from));
      break;
  }
}

bool PropertySet::EqualPropertyValues(const Property &property,
                                      const Effect &rhs) {
  assert(dynamic_cast<Effect *>(_object));
  const Effect &effect = static_cast<const Effect &>(*_object);
  assert(effect.GetType() == rhs.GetType());
  switch (property.type_) {
    case PropertyType::Boolean:
      return GetBool(property) == getBool(rhs, property.set_index_);
    case PropertyType::Choice:
      return GetChoice(property) == getChoice(rhs, property.set_index_);
    case PropertyType::ControlValue:
      return GetControlValue(property) ==
             getControlValue(rhs, property.set_index_);
    case PropertyType::Duration:
      return GetDuration(property) == getDuration(rhs, property.set_index_);
    case PropertyType::Integer:
      return GetInteger(property) == getInteger(rhs, property.set_index_);
    case PropertyType::TimePattern:
      return GetTimePattern(property) ==
             getTimePattern(rhs, property.set_index_);
    case PropertyType::Transition:
      return GetTransition(property) == getTransition(rhs, property.set_index_);
  }
  return false;
}

}  // namespace glight::theatre
