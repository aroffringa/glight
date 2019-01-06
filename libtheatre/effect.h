#ifndef EFFECT_H
#define EFFECT_H

#include "namedobject.h"

#include "../libtheatre/controllable.h"

#include <sigc++/connection.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

class Effect : public NamedObject
{
public:
	Effect(size_t controlCount) :
		_values(controlCount, 0),
		_controls(controlCount, nullptr)
	{
	}
	
	virtual ~Effect()
	{ 
		for(sigc::connection& c : _onDeleteConnections)
			c.disconnect();
	}
	
	std::vector<std::unique_ptr<class EffectControl>> ConstructControls();
	
	const std::vector<class EffectControl*>& Controls() const
	{ return _controls; }
	
	void AddConnection(Controllable* controllable)
	{
		_connections.emplace_back(controllable);
		_onDeleteConnections.emplace_back(
			controllable->SignalDelete().connect([&]() { RemoveConnection(controllable); })
		);
	}
	
	void RemoveConnection(Controllable* controllable)
	{
		std::vector<Controllable*>::iterator
			item = std::find(_connections.begin(), _connections.end(), controllable);
		if(item == _connections.end())
			throw std::runtime_error("RemoveConnection() called for unconnected controllable");
		// convert to index to also remove corresponding connection
		size_t index = item - _connections.begin();
		_connections.erase(item);
		_onDeleteConnections.erase(_onDeleteConnections.begin() + index);
	}
	
	const std::vector<Controllable*>& Connections() const { return _connections; }
	
protected:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const class Timing& timing) = 0;
	
private:
	friend class EffectControl;
	
	void setControlValue(size_t index, const ControlValue& value)
	{ _values[index] = value; }
	
	void collectAndMix(unsigned* channelValues, unsigned universe, const class Timing& timing)
	{
		mix(_values.data(), channelValues, universe, timing);
	}
	
	std::vector<ControlValue> _values;
	std::vector<class EffectControl*> _controls;
	std::vector<Controllable*> _connections;
	std::vector<sigc::connection> _onDeleteConnections;
};

#include "effectcontrol.h"

inline std::vector<std::unique_ptr<EffectControl>> Effect::ConstructControls()
{
#ifndef NDEBUG
	for(size_t i=0; i!=_controls.size(); ++i)
		if(_controls[i] != nullptr)
			throw std::runtime_error("Logical error");
#endif
	std::vector<std::unique_ptr<EffectControl>> controls;
	controls.reserve(_controls.size());
	for(size_t i=0; i!=_controls.size(); ++i)
	{
		controls.emplace_back(new EffectControl());
		EffectControl& control = *controls[i];
		control.attach(this, i, i == (_controls.size()-1));
	}
	return controls;
}

#endif
