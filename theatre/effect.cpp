#include "effect.h"

#include "effects/audioleveleffect.h"
#include "effects/delayeffect.h"
#include "effects/fadeeffect.h"
#include "effects/flickereffect.h"
#include "effects/inverteffect.h"
#include "effects/musicactivationeffect.h"
#include "effects/pulseeffect.h"
#include "effects/thresholdeffect.h"

#include "properties/propertyset.h"

std::unique_ptr<Effect> Effect::Make(Effect::Type type)
{
	using up = std::unique_ptr<Effect>;
	switch(type)
	{
		case AudioLevelType: return up(new AudioLevelEffect());
		case DelayType: return up(new DelayEffect());
		case FadeType: return up(new FadeEffect());
		case FlickerType: return up(new FlickerEffect());
		case InvertType: return up(new InvertEffect());
		case MusicActivationType: return up(new MusicActivationEffect());
		case PulseType: return up(new PulseEffect());
		case ThresholdType: return up(new ThresholdEffect());
	}
	return nullptr;
}

std::string Effect::TypeToName(Effect::Type type)
{
	switch(type)
	{
		case AudioLevelType: return "Audiolevel";
		case DelayType: return "Delay";
		case FadeType: return "Fade";
		case FlickerType: return "Flicker";
		case InvertType: return "Invert";
		case MusicActivationType: return  "Music activation";
		case PulseType: return "Pulse";
		case ThresholdType: return "Threshold";
	}
	return std::string();
}

Effect::Type Effect::NameToType(const std::string& name)
{
	if(name == "Audiolevel")
		return AudioLevelType;
	else if(name == "Delay")
		return DelayType;
	else if(name == "Fade")
		return FadeType;
	else if(name == "Flicker")
		return FlickerType;
	else if(name == "Invert")
		return InvertType;
	else if(name == "Music activation")
		return MusicActivationType;
	else if(name == "Pulse")
		return PulseType;
	else if(name == "Threshold")
		return ThresholdType;
	else
		throw std::runtime_error("Unknown effect type");
}

std::vector<Effect::Type> Effect::GetTypes()
{
	return std::vector<Effect::Type>{
		AudioLevelType,
		DelayType,
		FadeType,
		FlickerType,
		InvertType,
		MusicActivationType,
		PulseType,
		ThresholdType
	};
}

std::unique_ptr<Effect> Effect::Copy() const
{
	std::unique_ptr<Effect> copy = Make(GetType());
	std::unique_ptr<PropertySet>
		psSrc = PropertySet::Make(*this),
		psDest = PropertySet::Make(*copy);
	for(size_t i=0; i!=psSrc->size(); ++i)
	{
		psDest->AssignProperty((*psDest)[i], (*psSrc)[i], *psSrc);
	}
	copy->SetName(Name());
	return copy;
}
