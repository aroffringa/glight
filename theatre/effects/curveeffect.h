#ifndef CURVE_EFFECT_H
#define CURVE_EFFECT_H

#include "../effect.h"

#include <cmath>

class CurveEffect : public Effect
{
public:
	enum Function {
		Linear,
		Quadratic,
		Exponential,
		Logarithmic,
		Sinusoid,
		WarmUp,
		SquareRoot
	};
	
	CurveEffect() :
		Effect(1),
		_function(Exponential) // 2 %
	{ }
	
	virtual Effect::Type GetType() const override { return CurveType; }
	
	enum Function GetFunction() const { return _function; }
	void SetFunction(enum Function f) { _function = f; }
	
protected:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const class Timing& timing) final override
	{
		unsigned value = values[0].UInt();
		switch(_function)
		{
			case Linear:
				break;
			case Quadratic:
				value = ControlValue::MultiplyValues(value, value);
				break;
			case Exponential: {
				// bring value between 0 - 1
				double d = double(value) / ControlValue::MaxUInt();
				value = (std::exp(d*5.0) - 1.0) * ControlValue::MaxUInt() / (std::exp(5.0) - 1.0);
			} break;
			case Logarithmic: {
				// bring value between ~1 - 256
				double d = double(value) * 256.0 / ControlValue::MaxUInt();
				value = std::max(0.0, ControlValue::MaxUInt() * std::log(d) / std::log(256.0));
			} break;
			case Sinusoid: {
				// bring value between 0 - 1
				double d = double(value) / ControlValue::MaxUInt();
				value = (1.0-std::cos(d * M_PI)) * ControlValue::MaxUInt() / 2;
			} break;
			case WarmUp: {
				if(value < ControlValue::MaxUInt()*3/4)
					value /= 3;
				else
					value = value*3 - ControlValue::MaxUInt()*2;
			} break;
			case SquareRoot: {
				value = std::sqrt(double(value)) * std::sqrt(double(ControlValue::MaxUInt()));
			} break;
		}
		setConnectedInputs(ControlValue(value));
	}
private:
	Function _function;
};

#endif
