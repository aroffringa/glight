#ifndef INVERT_EFFECT_H
#define INVERT_EFFECT_H

#include "../effect.h"

#include <string>

class InvertEffect : public Effect
{
public:
	InvertEffect() :
		Effect(1),
		_offThreshold(ControlValue::MaxUInt()*2/100) // 2 %
	{ }
	
	virtual Effect::Type GetType() const override { return InvertType; }
	
	unsigned OffThreshold() const { return _offThreshold; }
	void SetOffThreshold(unsigned offThreshold) { _offThreshold = offThreshold; }
	
protected:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const class Timing& timing) final override
	{
		ControlValue inverted = ControlValue::MaxUInt() - values[0].UInt();
		if(inverted.UInt() < _offThreshold)
			inverted = ControlValue::Zero();
		for(const std::pair<Controllable*,size_t>& connection : Connections())
			connection.first->MixInput(connection.second, inverted);
	}
	
	virtual std::string getControlName(size_t) const final override { return Name() + "_M"; }
	
private:
	unsigned _offThreshold;
};

#endif
