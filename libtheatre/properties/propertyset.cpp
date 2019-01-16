#include "propertyset.h"

#include "audioleveleffectps.h"
#include "thresholdeffectps.h"

std::unique_ptr<PropertySet> PropertySet::Make(NamedObject& object)
{
	std::unique_ptr<PropertySet> ps;
	const AudioLevelEffect* afx = dynamic_cast<const AudioLevelEffect*>(&object);
	const ThresholdEffect* tfx = dynamic_cast<const ThresholdEffect*>(&object);
	if(tfx != nullptr)
	{
		ps.reset(new ThresholdEffectPS());
	}
	else if(afx != nullptr)
	{
		ps.reset(new AudioLevelEffectPS());
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

void PropertySet::AssignProperty(const Property& to, const Property& from, const PropertySet& fromSet)
{
	if(from._type != to._type)
		throw std::runtime_error("Copying different types");
	switch(from._type)
	{
	case Property::ControlValue:
		SetControlValue(to, fromSet.GetControlValue(from));
		break;
	}
}
