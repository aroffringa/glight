#include "controlwidget.h"

#include "../../theatre/presetvalue.h"

void ControlWidget::writeValue(unsigned target)
{
	_targetValue = target;
	double fadeSpeed =
		(_targetValue > _fadingValue) ? _fadeUpSpeed : _fadeDownSpeed;
	if(fadeSpeed == 0.0)
	{
		_fadingValue = _targetValue;
		if(Preset() != nullptr)
		{
			Preset()->Value().Set(_targetValue);
		}
	}
}

void ControlWidget::UpdateValue(double timePassed)
{
	if(_targetValue != _fadingValue)
	{
		double fadeSpeed =
			(_targetValue > _fadingValue) ? _fadeUpSpeed : _fadeDownSpeed;
		if(fadeSpeed == 0.0)
		{
			_fadingValue = _targetValue;
		}
		else {
			unsigned stepSize = unsigned(std::min<double>(timePassed * fadeSpeed * double(ControlValue::MaxUInt()+1), double(ControlValue::MaxUInt()+1)));
			if(_targetValue > _fadingValue)
			{
				if(_fadingValue + stepSize > _targetValue)
					_fadingValue = _targetValue;
				else
					_fadingValue += stepSize;
			}
			else {
				if(_targetValue + stepSize > _fadingValue)
					_fadingValue = _targetValue;
				else
					_fadingValue -= stepSize;
			}
		}
		if(Preset() != nullptr)
		{
			Preset()->Value().Set(_fadingValue);
		}
	}
}

double ControlWidget::MAX_SCALE_VALUE()
{
	return ControlValue::MaxUInt()+1;
}

