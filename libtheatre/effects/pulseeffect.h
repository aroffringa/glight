#ifndef PULSE_EFFECT_H
#define PULSE_EFFECT_H

#include "../effect.h"

class PulseEffect : public Effect
{
public:
	PulseEffect() :
		Effect(1),
		_isActive(false),
		_startTime(0),
		_repeat(false),
		_attack(300),
		_hold(200),
		_release(300),
		_sleep(200)
	{ }
	
	virtual Effect::Type GetType() const final override { return PulseType; }
	
	unsigned Attack() const { return _attack; }
	void SetAttack(unsigned attack) { _attack = attack; }
	
	unsigned Hold() const { return _hold; }
	void SetHold(unsigned hold) { _hold = hold; }
	
	unsigned Release() const { return _release; }
	void SetRelease(unsigned release) { _release = release; }
	
	unsigned Sleep() const { return _sleep; }
	void SetSleep(unsigned sleep) { _sleep = sleep; }
	
	bool Repeat() const { return _repeat; }
	void SetRepeat(bool repeat) { _repeat = repeat; }
	
protected:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const Timing& timing) final override
	{
	}
	
	virtual std::string getControlName(size_t) const final override
	{ return Name() + "_M"; }
private:
	bool _isActive;
	double _startTime;
	
	bool _repeat;
	// all in ms.
	unsigned _attack, _hold, _release, _sleep;
};

#endif
