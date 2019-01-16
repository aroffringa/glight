#ifndef AUDIO_LEVEL_EFFECT_H
#define AUDIO_LEVEL_EFFECT_H

#include "../effect.h"
#include "../timing.h"

#include <string>

class AudioLevelEffect final : public Effect
{
public:
	AudioLevelEffect() :
		Effect(1),
		_lastValue(0),
		_lastTime(0),
		_decaySpeed(((1<<24)-1)/100) // fully decay in 100 ms
	{ }
	
	virtual Effect::Type GetType() const final override { return AudioLevelType; }
	
	unsigned DecaySpeed() const { return _decaySpeed; }
	
	void SetDecaySpeed(unsigned decaySpeed) { _decaySpeed = decaySpeed; }

protected:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const Timing& timing) final override
	{
		unsigned audioLevel = (unsigned(timing.AudioLevel()) << 8);
		double timePassed = timing.TimeInMS() - _lastTime;
		_lastTime = timing.TimeInMS();
		unsigned decay = unsigned(std::min<double>(timePassed * _decaySpeed, (1<<24)-1));
		if(_lastValue < decay)
			_lastValue = 0;
		else
			_lastValue -= decay;
		_lastValue = std::max(_lastValue, audioLevel);
		
		unsigned v = ControlValue::Mix(_lastValue, values[0].UInt(), ControlValue::Multiply);
		ControlValue audioLevelCV(v);
		for(Controllable* connection : Connections())
		{
			connection->Mix(audioLevelCV, channelValues, universe, timing);
		}
	}
	
	virtual std::string getControlName(size_t) const final override { return Name() + "_M"; }
	
private:
	unsigned _lastValue;
	double _lastTime;
	
	// 2^24-1 means full slide every ms.
	unsigned _decaySpeed;
};

#endif

