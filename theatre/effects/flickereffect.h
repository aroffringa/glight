#ifndef FLICKER_EFFECT_H
#define FLICKER_EFFECT_H

#include "../effect.h"

#include <vector>

class FlickerEffect : public Effect
{
public:
	FlickerEffect() :
		Effect(1),
		_value(1, 0),
		_previousTime(0.0),
		_speed(ControlValue::MaxUInt()/200.0),
		_independentOutputs(true)
	{ };
	
	virtual Effect::Type GetType() const override { return FlickerType; }
	
	unsigned Speed() const { return _speed; }
	void SetSpeed(unsigned speed) { _speed = speed; }
	
	bool IndependentOutputs() const { return _independentOutputs; }
	void SetIndependentOutputs(bool independentOutputs) { _independentOutputs = independentOutputs; }
	
private:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const class Timing& timing) final override
	{
		if(values[0])
		{
			size_t count = _independentOutputs ? Connections().size() : 1;
			_value.resize(count);
			double delta = 2.0*(timing.TimeInMS() - _previousTime)*_speed/ControlValue::MaxUInt();
			_previousTime = timing.TimeInMS();
			
			for(size_t i=0; i!=count; ++i)
			{
				int rnd = int(timing.DrawRandomValue()) - int(ControlValue::MaxUInt()/2);
				int newValue = rnd * delta + int(_value[i]);
				if(newValue < 0)
					newValue = 0;
				else if(unsigned(newValue) > ControlValue::MaxUInt())
					newValue = ControlValue::MaxUInt();
				_value[i] = newValue;
			}
			
			if(_independentOutputs)
			{
				for(size_t i=0; i!=Connections().size(); ++i)
				{
					Connections()[i].first->MixInput(Connections()[i].second, ControlValue::MultiplyValues(values[0].UInt(), _value[i]));
				}
			}
			else {
				for(const std::pair<Controllable*,size_t>& connection : Connections())
					connection.first->MixInput(connection.second, ControlValue::MultiplyValues(values[0].UInt(), _value[0]));
			}
		}
	}
	
	std::vector<unsigned> _value;
	double _previousTime;
	/**
	 * Speed is in "control value travel per ms"
	 */
	unsigned _speed;
	bool _independentOutputs;
};

#endif
