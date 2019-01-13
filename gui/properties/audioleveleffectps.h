#ifndef AUDIO_LEVEL_EFFECT_PS
#define AUDIO_LEVEL_EFFECT_PS

#include "propertyset.h"

#include "../../libtheatre/effects/audioleveleffect.h"

class AudioLevelEffectPS final : public PropertySet
{
public:
	AudioLevelEffectPS()
	{
		addProperty(Property("Decay speed", Property::ControlValue));
	}
	
protected:
	virtual void setControlValue(NamedObject& object, size_t index, unsigned value) const final override
	{
		AudioLevelEffect& tfx = static_cast<AudioLevelEffect&>(object);
		switch(index) {
			case 0: tfx.SetDecaySpeed(value); break;
		}
	}
	
	virtual unsigned getControlValue(const NamedObject& object, size_t index) const final override
	{
		const AudioLevelEffect& tfx = static_cast<const AudioLevelEffect&>(object);
		switch(index) {
			case 0: return tfx.DecaySpeed();
		}
		return 0;
	}
};

#endif

