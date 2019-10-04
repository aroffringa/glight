#ifndef FLUORESCENT_START_EFFECT_H
#define FLUORESCENT_START_EFFECT_H

#include "../effect.h"
#include "../timing.h"

#include <vector>

class FluorescentStartEffect : public Effect
{
public:
	FluorescentStartEffect() :
		Effect(1),
		_averageDuration(1000.0), // 1 sec
		_stdDeviation(500.0),
		_flashDuration(80.0),
		_glowValue(ControlValue::MaxUInt()/5), // 20%
		_independentOutputs(true)
	{ };
	
	virtual Effect::Type GetType() const override { return FlickerType; }
	
	double AverageDuration() const { return _averageDuration; }
	void SetAverageDuration(double avgDuration) { _averageDuration = avgDuration; }
	
	double StdDeviation() const { return _stdDeviation; }
	void SetStdDeviation(double stdDev) { _stdDeviation = stdDev; }

	double FlashDuration() const { return _flashDuration; }
	void SetFlashDuration(double duration) { _flashDuration = duration; }
	
	unsigned GlowValue() const { return _glowValue; }
	void SetGlowValue(unsigned glowValue) { _glowValue = glowValue; }
	
	bool IndependentOutputs() const { return _independentOutputs; }
	void SetIndependentOutputs(bool independentOutputs) { _independentOutputs = independentOutputs; }
	
private:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const class Timing& timing) final override
	{
		if(values[0])
		{
			size_t count = _independentOutputs ? Connections().size() : 1;
			_data.resize(count);
			
			for(size_t i=0; i!=count; ++i)
			{
				ConnectionInfo& data = _data[i];
				bool nextState =
					(data._state != std::numeric_limits<unsigned>::max()) &&
					( (data._state == 0) || (data._nextStateTime < timing.TimeInMS()) );
				if(nextState)
				{
					++data._state;
					if(data._state >= 6)
						data._state = std::numeric_limits<unsigned>::max();
					else {
						if(data._state%2 == 0) // flash?
							data._nextStateTime = timing.TimeInMS() + _flashDuration;
						else
							data._nextStateTime = timing.DrawGaussianValue() * _stdDeviation + _averageDuration + timing.TimeInMS();
					}
				}
				unsigned value;
				if(data._state%2 == 0 || data._state==std::numeric_limits<unsigned>::max())
					value = ControlValue::MaxUInt();
				else if(data._state==1)
					value = 0;
				else
					value = _glowValue;
				if(_independentOutputs)
				{
					Connections()[i].first->MixInput(Connections()[i].second, ControlValue::MultiplyValues(values[0].UInt(), value));
				}
				else {
					setConnectedInputs(ControlValue::MultiplyValues(values[0].UInt(), value));
				}
			}
		}
		else {
			_data.clear();
		}
	}
	
	struct ConnectionInfo {
		ConnectionInfo() :
			_nextStateTime(0.0),
			_state(0)
		{ }
		double _nextStateTime;
		// 0 means off, 1 means in first dark interval, 2 first flash, 3 second dark interval...
		// max means on.
		unsigned _state;
	};
	
	std::vector<ConnectionInfo> _data;
	double _averageDuration, _stdDeviation, _flashDuration;
	size_t _glowValue;
	bool _independentOutputs;
};

#endif
