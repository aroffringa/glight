#ifndef MUSIC_ACTIVATION_EFFECT_PS
#define MUSIC_ACTIVATION_EFFECT_PS

#include "propertyset.h"

#include "../../theatre/effects/musicactivationeffect.h"

class MusicActivationEffectPS final : public PropertySet
{
public:
	MusicActivationEffectPS()
	{
		addProperty(Property("offdelay", "Off delay", Property::Duration));
	}
	
protected:
	virtual void setDuration(FolderObject& object, size_t index, double value) const override
	{
		MusicActivationEffect& mfx = static_cast<MusicActivationEffect&>(object);
		switch(index) {
			case 0: mfx.SetOffDelay(value); break;
		}
	}
	
	virtual double getDuration(const FolderObject& object, size_t index) const final override
	{
		const MusicActivationEffect& mfx = static_cast<const MusicActivationEffect&>(object);
		switch(index) {
			case 0: return mfx.OffDelay();
		}
		return 0.0;
	}
};

#endif


