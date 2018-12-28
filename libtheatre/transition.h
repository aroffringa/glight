#ifndef TRANSITION_H
#define TRANSITION_H

#include "presetcollection.h"

/**
	@author Andre Offringa
*/
class Transition {
	public:
		enum Type { None, Fade, FadeWithBlack };

		Transition() : _lengthInMs(250.0), _type(Fade) { }
		~Transition() { }

		enum Type Type() const { return _type; }
		void SetType(enum Type type) { _type = type; }

		double LengthInMs() const { return _lengthInMs; }
		void SetLengthInMs(double length) { _lengthInMs = length; }

		void Mix(PresetCollection &first, PresetCollection &second, double transitionTime, const ControlValue &value, unsigned *channelValues, unsigned universe, const Timing& timing)
		{
			switch(_type)
			{
				case None:
					if(transitionTime * 2.0 <= _lengthInMs)
						first.Mix(value, channelValues, universe, timing);
					else
						second.Mix(value, channelValues, universe, timing);
				break;
				case Fade:
				{
					unsigned secondChannelValues[512];
					for(unsigned i=0;i<512;++i)
						secondChannelValues[i] = channelValues[i];
					first.Mix(value, channelValues, universe, timing);
					second.Mix(value, secondChannelValues, universe, timing);
					unsigned secondRatioValue = (unsigned) ((transitionTime / _lengthInMs) * 256.0);
					unsigned firstRatioValue = 255 - secondRatioValue;
					for(unsigned i=0;i<512;++i)
					{
						channelValues[i] = 
								(channelValues[i] * firstRatioValue +
								secondChannelValues[i] * secondRatioValue)
							>> 8;
					}
				}
				break;
				case FadeWithBlack:
				{
					unsigned ratio = (unsigned) ((transitionTime / _lengthInMs) * 512.0);
					if(ratio < 256)
					{
						ControlValue firstValue((value.UInt() * (255 - ratio)) >> 8);
						first.Mix(firstValue, channelValues, universe, timing);
					}
					else
					{
						ControlValue secondValue((value.UInt() * (ratio-256)) >> 8);
						second.Mix(secondValue, channelValues, universe, timing);
					}
				}
				break;
			}
		}
	private:
		double _lengthInMs;
		enum Type _type;
};

#endif
