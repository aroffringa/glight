#include "propertyset.h"

#include "audioleveleffectps.h"
#include "delayeffectps.h"
#include "fadeeffectps.h"
#include "inverteffectps.h"
#include "pulseeffectps.h"
#include "thresholdeffectps.h"

std::unique_ptr<PropertySet> PropertySet::Make(FolderObject& object)
{
	std::unique_ptr<PropertySet> ps;
	const AudioLevelEffect* afx = dynamic_cast<const AudioLevelEffect*>(&object);
	const DelayEffect* dfx = dynamic_cast<const DelayEffect*>(&object);
	const FadeEffect* ffx = dynamic_cast<const FadeEffect*>(&object);
	const InvertEffect* ifx = dynamic_cast<const InvertEffect*>(&object);
	const PulseEffect* pfx = dynamic_cast<const PulseEffect*>(&object);
	const ThresholdEffect* tfx = dynamic_cast<const ThresholdEffect*>(&object);
	if(afx != nullptr)
	{
		ps.reset(new AudioLevelEffectPS());
	}
	else if(dfx != nullptr)
	{
		ps.reset(new DelayEffectPS());
	}
	else if(ffx != nullptr)
	{
		ps.reset(new FadeEffectPS());
	}
	else if(ifx != nullptr)
	{
		ps.reset(new InvertEffectPS());
	}
	else if(pfx != nullptr)
	{
		ps.reset(new PulseEffectPS());
	}
	else if(tfx != nullptr)
	{
		ps.reset(new ThresholdEffectPS());
	}
	else {
		throw std::runtime_error("Unknown object type specified in call to PropertySet::Make()");
	}
	ps->_object = &object;
	return ps;
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
	case Property::Duration:
		SetDuration(to, fromSet.GetDuration(from));
		break;
	case Property::Boolean:
		SetBool(to, fromSet.GetBool(from));
		break;
	}
}
