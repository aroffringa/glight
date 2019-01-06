#ifndef EFFECT_CONTROL_H
#define EFFECT_CONTROL_H

#include "controllable.h"

class EffectControl : public Controllable
{
public:
	EffectControl() : _effect(nullptr), _isLast(false) { }
	
	class Effect& GetEffect();
	
	virtual void Mix(const ControlValue& value, unsigned* channelValues, unsigned universe, const class Timing& timing) final override;
private:
	friend class Effect;
	void attach(class Effect* effect, size_t index, bool isLast)
	{
		_effect = effect;
		_index = index;
		_isLast = isLast;
	}
	class Effect* _effect;
	size_t _index;
	bool _isLast;
};

#include "effect.h"

inline void EffectControl::Mix(const ControlValue& value, unsigned* channelValues, unsigned universe, const class Timing& timing)
{ 
	_effect->setControlValue(_index, value);
	if(_isLast)
		_effect->collectAndMix(channelValues, universe, timing);
}

#endif

