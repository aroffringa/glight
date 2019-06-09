#ifndef FADE_EFFECT_H
#define FADE_EFFECT_H

#include "../effect.h"
#include "../timing.h"

class FadeEffect : public Effect
{
public:
	FadeEffect() :
		Effect(1),
		_fadingValue(0),
		_fadeUpSpeed(1.0),
		_fadeDownSpeed(1.0),
		_sustain(0.0),
		_previousTime(0.0),
		_sustainTimer(0.0)
	{ }
	
	virtual Effect::Type GetType() const override { return FadeType; }
	
	double FadeUpSpeed() const { return _fadeUpSpeed; }
	void SetFadeUpSpeed(double speed) { _fadeUpSpeed = speed; }
	
	double FadeDownSpeed() const { return _fadeDownSpeed; }
	void SetFadeDownSpeed(double speed) { _fadeDownSpeed = speed; }
	
	double Sustain() const { return _sustain; }
	void SetSustain(double sustain) { _sustain = sustain; }
	
protected:
	virtual void mix(const ControlValue* values, unsigned*, unsigned, const Timing& timing) final override
	{
		double timePassed = 0.001*(timing.TimeInMS() - _previousTime);
		_previousTime = timing.TimeInMS();
		unsigned targetValue = values[0].UInt();
		if(targetValue != _fadingValue)
		{
			if(targetValue > _fadingValue)
			{
				_sustainTimer = _sustain;
				if(_fadeUpSpeed == 0.0)
					_fadingValue = targetValue;
				else {
					unsigned stepSize = unsigned(std::min<double>(timePassed * _fadeUpSpeed * double(ControlValue::MaxUInt()), double(ControlValue::MaxUInt())));
					if(_fadingValue + stepSize > targetValue)
						_fadingValue = targetValue;
					else
						_fadingValue += stepSize;
				}
			}
			else {
				if(_sustainTimer > timePassed)
					_sustainTimer -= timePassed;
				else {
					timePassed -= _sustainTimer;
					_sustainTimer = 0.0;
					if(_fadeDownSpeed == 0.0)
						_fadingValue = targetValue;
					else {
						unsigned stepSize = unsigned(std::min<double>(timePassed * _fadeDownSpeed * double(ControlValue::MaxUInt()), double(ControlValue::MaxUInt())));
						if(targetValue + stepSize > _fadingValue)
							_fadingValue = targetValue;
						else
							_fadingValue -= stepSize;
					}
				}
			}
		}
		if(_fadingValue != 0)
		{
			setConnectedInputs(ControlValue(_fadingValue));
		}
	}
	
	virtual std::string getControlName(size_t) const final override { return Name() + "_M"; }
	
private:
	unsigned _fadingValue;
	double _fadeUpSpeed, _fadeDownSpeed;
	double _sustain;
	double _previousTime, _sustainTimer;
};

#endif
