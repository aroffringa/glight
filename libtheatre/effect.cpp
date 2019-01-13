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

std::string Effect::TypeName(Effect::Type type)
{
	switch(type)
	{
		case AudioLevelType: return "Audio level";
		case ThresholdType: return "Threshold";
	}
	return std::string();
}
