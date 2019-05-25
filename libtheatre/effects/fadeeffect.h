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
		_previousTime(0.0)
	{ }
	
	virtual Effect::Type GetType() const override { return FadeType; }
	
	double FadeUpSpeed() const { return _fadeUpSpeed; }
	void SetFadeUpSpeed(double speed) { _fadeUpSpeed = speed; }
	
	double FadeDownSpeed() const { return _fadeDownSpeed; }
	void SetFadeDownSpeed(double speed) { _fadeDownSpeed = speed; }
	
protected:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const Timing& timing) final override
	{
		double timePassed = 0.001*(timing.TimeInMS() - _previousTime);
		_previousTime = timing.TimeInMS();
		unsigned targetValue = values[0].UInt();
		if(targetValue != _fadingValue)
		{
			double fadeSpeed =
				(targetValue > _fadingValue) ? _fadeUpSpeed : _fadeDownSpeed;
			if(fadeSpeed == 0.0)
			{
				_fadingValue = targetValue;
			}
			else {
				unsigned stepSize = unsigned(std::min<double>(timePassed * fadeSpeed * double(ControlValue::MaxUInt()), double(ControlValue::MaxUInt())));
				if(targetValue > _fadingValue)
				{
					if(_fadingValue + stepSize > targetValue)
						_fadingValue = targetValue;
					else
						_fadingValue += stepSize;
				}
				else {
					if(targetValue + stepSize > _fadingValue)
						_fadingValue = targetValue;
					else
						_fadingValue -= stepSize;
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
	double _previousTime;
};

#endif
