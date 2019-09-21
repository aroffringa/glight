#ifndef RANDOM_EFFECT_H
#define RANDOM_EFFECT_H

#include "../effect.h"
#include "../timing.h"

#include <vector>

class RandomSelectEffect : public Effect
{
public:
	RandomSelectEffect() :
		Effect(1),
		_active(false),
		_startTime(0.0),
		_delay(10000.0),
		_count(1)
	{ };
	
	virtual Effect::Type GetType() const override { return RandomSelectType; }
	
	double Delay() const { return _delay; }
	void SetDelay(double delay) { _delay = delay; }
	
	size_t Count() const { return _count; }
	void SetCount(size_t count) { _count = count; }
	
private:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const class Timing& timing) final override
	{
		size_t count = std::min(_count, Connections().size());
		if(values[0] && count!=0)
		{
			if(Connections().size() != _activeConnections.size())
			{
				_activeConnections.resize(Connections().size());
				for(size_t i=0; i!=_activeConnections.size(); ++i)
					_activeConnections[i] = i;
				std::shuffle(_activeConnections.begin(), _activeConnections.end(), timing.RNG());
			}
			if(!_active || timing.TimeInMS() - _startTime > _delay)
			{
				_active = true;
				_startTime = timing.TimeInMS();
				std::shuffle(_activeConnections.begin(), _activeConnections.end(), timing.RNG());
			}
			for(size_t i=0; i!=count; ++i)
			{
				if(_activeConnections[i] < Connections().size())
				{
					const std::pair<Controllable*,size_t>& connection = Connections()[_activeConnections[i]];
					connection.first->MixInput(connection.second, values[0].UInt());
				}
			}
		}
		else {
			_active = false;
		}
	}
	
	bool _active;
	std::vector<size_t> _activeConnections;
	double _startTime;
	double _delay;
	size_t _count;
};

#endif

