#include "propertyset.h"

#include "audioleveleffectps.h"
#include "constantvalueeffectps.h"
#include "delayeffectps.h"
#include "fadeeffectps.h"
#include "flickereffectps.h"
#include "inverteffectps.h"
#include "musicactivationeffectps.h"
#include "pulseeffectps.h"
#include "randomselecteffectps.h"
#include "thresholdeffectps.h"

std::unique_ptr<PropertySet> PropertySet::Make(FolderObject& object)
{
	std::unique_ptr<PropertySet> ps;
	const AudioLevelEffect* afx = dynamic_cast<const AudioLevelEffect*>(&object);
	const ConstantValueEffect* cfx = dynamic_cast<const ConstantValueEffect*>(&object);
	const DelayEffect* dfx = dynamic_cast<const DelayEffect*>(&object);
	const FadeEffect* ffx = dynamic_cast<const FadeEffect*>(&object);
	const FlickerEffect* flx = dynamic_cast<const FlickerEffect*>(&object);
	const InvertEffect* ifx = dynamic_cast<const InvertEffect*>(&object);
	const MusicActivationEffect* mfx = dynamic_cast<const MusicActivationEffect*>(&object);
	const PulseEffect* pfx = dynamic_cast<const PulseEffect*>(&object);
	const RandomSelectEffect* rfx = dynamic_cast<const RandomSelectEffect*>(&object);
	const ThresholdEffect* tfx = dynamic_cast<const ThresholdEffect*>(&object);
	if(afx != nullptr)
	{
		ps.reset(new AudioLevelEffectPS());
	}
	else if(cfx != nullptr)
	{
		ps.reset(new ConstantValueEffectPS());
	}
	else if(dfx != nullptr)
	{
		ps.reset(new DelayEffectPS());
	}
	else if(ffx != nullptr)
	{
		ps.reset(new FadeEffectPS());
	}
	else if(flx != nullptr)
	{
		ps.reset(new FlickerEffectPS());
	}
	else if(ifx != nullptr)
	{
		ps.reset(new InvertEffectPS());
	}
	else if(mfx != nullptr)
	{
		ps.reset(new MusicActivationEffectPS());
	}
	else if(pfx != nullptr)
	{
		ps.reset(new PulseEffectPS());
	}
	else if(rfx != nullptr)
	{
		ps.reset(new RandomSelectEffectPS());
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
	case Property::Boolean:
		SetBool(to, fromSet.GetBool(from));
		break;
	case Property::ControlValue:
		SetControlValue(to, fromSet.GetControlValue(from));
		break;
	case Property::Duration:
		SetDuration(to, fromSet.GetDuration(from));
		break;
	case Property::Integer:
		SetInteger(to, fromSet.GetInteger(from));
		break;
	}
}
