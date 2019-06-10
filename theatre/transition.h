#ifndef TRANSITION_H
#define TRANSITION_H

#include "presetcollection.h"

/**
	@author Andre Offringa
*/
class Transition {
	public:
		enum Type { None, Fade, FadeThroughBlack, Erratic };

		Transition() : _lengthInMs(250.0), _type(Fade) { }
		~Transition() { }

		enum Type Type() const { return _type; }
		void SetType(enum Type type) { _type = type; }

		double LengthInMs() const { return _lengthInMs; }
		void SetLengthInMs(double length) { _lengthInMs = length; }

		/**
		 * @param transitionTime value between 0 and _lengthInMS.
		 */
		void Mix(Controllable &first, size_t firstInput, Controllable &second, size_t secondInput, double transitionTime, const ControlValue &value, const Timing& timing) const
		{
			switch(_type)
			{
				case None:
					if(transitionTime * 2.0 <= _lengthInMs)
						first.MixInput(firstInput, value);
					else
						second.MixInput(secondInput, value);
				break;
				case Fade:
				{
					unsigned secondRatioValue = (unsigned) ((transitionTime / _lengthInMs) * 256.0);
					unsigned firstRatioValue = 255 - secondRatioValue;
					first.MixInput(firstInput, (value.UInt()*firstRatioValue) >> 8);
					second.MixInput(secondInput, (value.UInt()*secondRatioValue) >> 8);
				}
				break;
				case FadeThroughBlack:
				{
					unsigned ratio = (unsigned) ((transitionTime / _lengthInMs) * 512.0);
					if(ratio < 256)
					{
						ControlValue firstValue((value.UInt() * (255 - ratio)) >> 8);
						first.MixInput(firstInput, firstValue);
					}
					else
					{
						ControlValue secondValue((value.UInt() * (ratio-256)) >> 8);
						second.MixInput(secondInput, secondValue);
					}
				}
				break;
				case Erratic:
				{
					unsigned ratio = (unsigned) ((transitionTime / _lengthInMs) * ControlValue::MaxUInt());
					if(ratio < timing.DrawRandomValue())
						first.MixInput(firstInput, value);
					else
						second.MixInput(secondInput, value);
				}
			}
		}
	private:
		double _lengthInMs;
		enum Type _type;
};

#endif
