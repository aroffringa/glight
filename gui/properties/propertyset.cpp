#include "propertyset.h"

#include "thresholdeffectps.h"

std::unique_ptr<PropertySet> PropertySet::Make(const Effect& object)
{
	using up = std::unique_ptr<PropertySet>;
	const ThresholdEffect* tfx = dynamic_cast<const ThresholdEffect*>(&object);
	if(tfx != nullptr)
		return up(new ThresholdEffectPS());
	throw std::runtime_error("Unknown object type specified in call to PropertySet::Make()");
}
