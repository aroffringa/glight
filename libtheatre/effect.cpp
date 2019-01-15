#include "effect.h"

#include "effects/audioleveleffect.h"
#include "effects/thresholdeffect.h"

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
