#include "effect.h"

#include "effects/audioleveleffect.h"
#include "effects/thresholdeffect.h"

#include "properties/propertyset.h"

std::unique_ptr<Effect> Effect::Make(Effect::Type type)
{
	using up = std::unique_ptr<Effect>;
	switch(type)
	{
		case AudioLevelType: return up(new AudioLevelEffect());
		case ThresholdType: return up(new ThresholdEffect());
	}
	return nullptr;
}

std::string Effect::TypeToName(Effect::Type type)
{
	switch(type)
	{
		case AudioLevelType: return "Audiolevel";
		case ThresholdType: return "Threshold";
	}
	return std::string();
}

Effect::Type Effect::NameToType(const std::string& name)
{
	if(name == "Audiolevel")
		return AudioLevelType;
	else if(name == "Threshold")
		return ThresholdType;
	else
		throw std::runtime_error("Unknown effect type");
}

void Effect::SetNameGlobally(const std::string& effectName)
{
	SetName(effectName);
	for(size_t i=0; i!=_controls.size(); ++i)
	{
		if(_controls[i] != nullptr)
			_controls[i]->SetName(getControlName(i));
	}
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
	return std::move(copy);
}