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
		_lowerStartLimit(ControlValue::MaxUInt()*1/3),
		_lowerEndLimit(ControlValue::MaxUInt()*2/3),
		_upperStartLimit(ControlValue::MaxUInt()),
		_upperEndLimit(ControlValue::MaxUInt())
	{ }
	
	virtual Effect::Type GetType() const override { return ThresholdType; }
	
	unsigned LowerStartLimit() const { return _lowerStartLimit; }
	unsigned LowerEndLimit() const { return _lowerEndLimit; }
	unsigned UpperStartLimit() const { return _upperStartLimit; }
	unsigned UpperEndLimit() const { return _upperEndLimit; }
	void SetLowerStartLimit(unsigned lowerStartLimit) { _lowerStartLimit = lowerStartLimit; }
	void SetLowerEndLimit(unsigned lowerEndLimit) { _lowerEndLimit = lowerEndLimit; }
	void SetUpperStartLimit(unsigned upperStartLimit) { _upperStartLimit = upperStartLimit; }
	void SetUpperEndLimit(unsigned upperEndLimit) { _upperEndLimit = upperEndLimit; }
	
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
		for(const std::pair<Controllable*,size_t>& connection : Connections())
		{
			connection.first->MixInput(connection.second, thresholded);
		}
	}
	
	virtual std::string getControlName(size_t) const final override { return Name() + "_M"; }
	
private:
	unsigned
		_lowerStartLimit, _lowerEndLimit,
		_upperStartLimit, _upperEndLimit;
};

#endif
