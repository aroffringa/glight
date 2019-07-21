#ifndef DELAY_EFFECT_PS
#define DELAY_EFFECT_PS

#include "propertyset.h"

#include "../effects/delayeffect.h"

class DelayEffectPS final : public PropertySet
{
public:
	DelayEffectPS()
	{
		addProperty(Property("delay", "Delay", Property::Duration));
	}
	
protected:
	virtual void setDuration(FolderObject& object, size_t index, double value) const final override
	{
		DelayEffect& dfx = static_cast<DelayEffect&>(object);
		switch(index) {
			case 0: dfx.SetDelayInMS(value); break;
		}
	}
	
	virtual double getDuration(const FolderObject& object, size_t index) const final override
	{
		const DelayEffect& dfx = static_cast<const DelayEffect&>(object);
		switch(index) {
			case 0: return dfx.DelayInMS();
		}
		return 0;
	}
};

#endif

