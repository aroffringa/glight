#ifndef THRESHOLD_EFFECT_H
#define THRESHOLD_EFFECT_H

#include "../effect.h"

#include <string>

/**
 * This effect applies the following transformation to the control value:
 * 
 *  o       /--\ 
 *  u      /    \ 
 *  t  ___/      \ __
 *   input
 *    0   A B  C D   max
 * 
 * A, B, C and D are the lower start, lower end, upper start and upper end
 * limits respectively. By setting C and D to max, the decreasing part can be remove.
 */
class ThresholdEffect : public Effect
{
public:
	ThresholdEffect() :
		Effect(1),
		_lowerStartLimit(ControlValue::Max().UInt()*1/3),
		_lowerEndLimit(ControlValue::Max().UInt()*2/3),
		_upperStartLimit(ControlValue::Max().UInt()+1),
		_upperEndLimit(ControlValue::Max().UInt()+1)
	{ }
	
protected:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const class Timing& timing) final override
	{
		ControlValue thresholded;
		if(values[0].UInt() < _lowerEndLimit)
		{
			if(values[0].UInt() <= _lowerStartLimit)
				thresholded.Set(0);
			else { //  lowerstart < value < lowerend
				unsigned v = (values[0].UInt() - _lowerStartLimit) * 255 / (_lowerEndLimit - _lowerStartLimit);
				thresholded.Set(v * 65536);
			}
		}
		else { // value >= lowerend
			if(values[0].UInt() <= _upperStartLimit)
				thresholded = ControlValue::Max();
			else if(values[0].UInt() > _upperEndLimit)
				thresholded.Set(0);
			else { // upperend >= value > upperstart
				unsigned v = (values[0].UInt() - _upperStartLimit) * 255 / (_lowerEndLimit - _lowerStartLimit);
				thresholded.Set(ControlValue::Max().UInt() - v * 65536);
			}
		}
		for(Controllable* connection : Connections())
		{
			connection->Mix(thresholded, channelValues, universe, timing);
		}
	}
	
	virtual std::string getControlName(size_t index) const final override { return Name(); }
	
private:
	unsigned
		_lowerStartLimit, _lowerEndLimit,
		_upperStartLimit, _upperEndLimit;
};

#endif
