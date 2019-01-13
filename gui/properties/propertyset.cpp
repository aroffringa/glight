#include "propertyset.h"

#include "thresholdeffectps.h"

std::unique_ptr<PropertySet> PropertySet::Make(NamedObject& object)
{
	std::unique_ptr<PropertySet> ps;
	const ThresholdEffect* tfx = dynamic_cast<const ThresholdEffect*>(&object);
	if(tfx != nullptr)
	{
		ps.reset(new ThresholdEffectPS());
	}
	else {
		throw std::runtime_error("Unknown object type specified in call to PropertySet::Make()");
	}
	ps->_object = &object;
	return ps;
}

std::string PropertySet::GetTypeDescription() const
{
	if(dynamic_cast<const ThresholdEffect*>(_object))
		return "Threshold effect";
	else
		return "Unknown object";
}
