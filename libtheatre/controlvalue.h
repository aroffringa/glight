#ifndef CONTROLVALUE_H
#define CONTROLVALUE_H

/**
	@author Andre Offringa
*/
class ControlValue {
	public:
		enum MixStyle { Default, HighestValue, Sum, LowestValue, Multiply, First, Second };

		ControlValue(unsigned value) { _value = value; }
		ControlValue(const ControlValue &source) { _value = source._value; }
		~ControlValue() { }

		unsigned int UInt() const { return _value; }

		static ControlValue Zero()
		{
			return ControlValue(0);
		}
		static ControlValue Max()
		{
			return ControlValue((1<<24)-1);
		}

		static unsigned Mix(unsigned firstValue, unsigned secondValue, MixStyle mixStyle)
		{
			switch(mixStyle)
			{
				default:
				case Default:
				case HighestValue:
					if(firstValue > secondValue)
						return firstValue;
					else
						return secondValue;
				case Sum:
					return firstValue + secondValue;
				case LowestValue:
					if(firstValue > secondValue)
						return secondValue;
					else
						return firstValue;
				case Multiply:
					firstValue >>= 9;
					secondValue >>= 9;
					return (firstValue * secondValue) >> 6;
				case First:
					return firstValue;
				case Second:
					return secondValue;
			}
		}
		static MixStyle CombineMixStyles(MixStyle primaryStyle, MixStyle secondaryStyle)
		{
			if(primaryStyle == Default)
				return secondaryStyle;
			else
				return primaryStyle;
		}
		void Set(unsigned int uintValue) { _value = uintValue; }
		double Ratio() const
		{
			return (double) _value / (double) ((1<<24)-1);
		}
	private:
		unsigned int _value;
};

typedef ControlValue::MixStyle MixStyle;

#endif
