#ifndef FADE_EFFECT_PS
#define FADE_EFFECT_PS

#include "propertyset.h"

#include "../effects/fadeeffect.h"

class FadeEffectPS final : public PropertySet
{
public:
	FadeEffectPS()
	{
		addProperty(Property("upspeed", "Up fading speed", Property::Duration));
		addProperty(Property("downspeed", "Down fading speed", Property::Duration));
		addProperty(Property("sustaintime", "Sustain time", Property::Duration));
	}
	
protected:
	virtual void setDuration(FolderObject& object, size_t index, double value) const final override
	{
		FadeEffect& fadefx = static_cast<FadeEffect&>(object);
		switch(index) {
			case 0: fadefx.SetFadeUpSpeed(value); break;
			case 1: fadefx.SetFadeDownSpeed(value); break;
			case 2: fadefx.SetSustain(value); break;
		}
	}
	
	virtual double getDuration(const FolderObject& object, size_t index) const final override
	{
		const FadeEffect& fadefx = static_cast<const FadeEffect&>(object);
		switch(index) {
			case 0: return fadefx.FadeUpSpeed();
			case 1: return fadefx.FadeDownSpeed();
			case 2: return fadefx.Sustain();
		}
		return 0;
	}
};

#endif

