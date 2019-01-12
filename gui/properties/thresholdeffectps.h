#ifndef EFFECT_PS
#define EFFECT_PS

#include "propertyset.h"

class ThresholdEffectPS final : public PropertySet
{
public:
	ThresholdEffectPS()
	{
		addProperty(Property("Lower start limit", Property::ControlValue));
		addProperty(Property("Upper start limit", Property::ControlValue));
		addProperty(Property("Lower end limit", Property::ControlValue));
		addProperty(Property("Upper end limit", Property::ControlValue));
	}
	
protected:
	virtual void setControlValue(NamedObject& object, size_t index, unsigned value) const final override
	{
		ThresholdEffect& tfx = static_cast<ThresholdEffect&>(object);
		switch(index) {
			case 0: tfx.SetLowerStartLimit(value); break;
			case 1: tfx.SetLowerEndLimit(value); break;
			case 2: tfx.SetUpperStartLimit(value); break;
			case 3: tfx.SetUpperEndLimit(value); break;
		}
	}
	
	virtual unsigned getControlValue(const NamedObject& object, size_t index) const final override
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
