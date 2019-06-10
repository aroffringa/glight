#ifndef THRESHOLD_EFFECT_PS
#define THRESHOLD_EFFECT_PS

#include "propertyset.h"

#include "../effects/thresholdeffect.h"

class ThresholdEffectPS final : public PropertySet
{
public:
	ThresholdEffectPS()
	{
		addProperty(Property("lowerstartlim", "Lower start limit", Property::ControlValue));
		addProperty(Property("upperstartlim", "Upper start limit", Property::ControlValue));
		addProperty(Property("lowerendlim", "Lower end limit", Property::ControlValue));
		addProperty(Property("upperendlim", "Upper end limit", Property::ControlValue));
	}
	
protected:
	virtual void setControlValue(FolderObject& object, size_t index, unsigned value) const final override
	{
		ThresholdEffect& tfx = static_cast<ThresholdEffect&>(object);
		switch(index) {
			case 0: tfx.SetLowerStartLimit(value); break;
			case 1: tfx.SetLowerEndLimit(value); break;
			case 2: tfx.SetUpperStartLimit(value); break;
			case 3: tfx.SetUpperEndLimit(value); break;
		}
	}
	
	virtual unsigned getControlValue(const FolderObject& object, size_t index) const final override
	{
		const ThresholdEffect& tfx = static_cast<const ThresholdEffect&>(object);
		switch(index) {
			case 0: return tfx.LowerStartLimit();
			case 1: return tfx.LowerEndLimit();
			case 2: return tfx.UpperStartLimit();
			case 3: return tfx.UpperEndLimit();
		}
		return 0;
	}
};

#endif
