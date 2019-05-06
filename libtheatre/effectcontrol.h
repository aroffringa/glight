#ifndef EFFECT_CONTROL_H
#define EFFECT_CONTROL_H

#include "controllable.h"

class EffectControl : public Controllable
{
public:
	EffectControl() :
		_effect(nullptr),
		_index(0)
	{ }
	
	class Effect& GetEffect() { return *_effect; }
	const class Effect& GetEffect() const { return *_effect; }
	
	virtual void Mix(const ControlValue& value, unsigned* channelValues, unsigned universe, const class Timing& timing) final override;
	
private:
	friend class Effect;
	void attach(class Effect* effect, size_t index)
	{
		_effect = effect;
		_index = index;
	}
	class Effect* _effect;
	size_t _index;
};

#include "effect.h"

inline void EffectControl::Mix(const ControlValue& value, unsigned*, unsigned, const class Timing&)
{ 
	_effect->mixControlValue(_index, value);
}

#endif

