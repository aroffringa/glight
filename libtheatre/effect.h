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
	enum Type {
		AudioLevelType,
		DelayType,
		ThresholdType
	};
	
	Effect(size_t controlCount) :
		_values(controlCount, 0),
		_nValuesSet(0),
		_controls(controlCount, nullptr)
	{
	}
	
	virtual ~Effect()
	{ 
		for(sigc::connection& c : _onDeleteConnections)
			c.disconnect();
	}
	
	virtual Type GetType() const = 0;
	
	static std::unique_ptr<Effect> Make(Type type);
	
	static std::string TypeToName(Type type);
	
	static Type NameToType(const std::string& name);
	
	static std::vector<Type> GetTypes();
	
	std::vector<std::unique_ptr<class EffectControl>> ConstructControls();
	
	const std::vector<class EffectControl*>& Controls() const
	{ return _controls; }
	
	void AddConnection(Controllable* controllable)
	{
		_connections.emplace_back(controllable);
		_onDeleteConnections.emplace_back(
			controllable->SignalDelete().connect([controllable,this]() { RemoveConnection(controllable); })
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
		_onDeleteConnections[index].disconnect();
		_onDeleteConnections.erase(_onDeleteConnections.begin() + index);
	}
	
	void RemoveConnection(size_t index)
	{
		_connections.erase(_connections.begin() + index);
		_onDeleteConnections[index].disconnect();
		_onDeleteConnections.erase(_onDeleteConnections.begin() + index);
	}
	
	const std::vector<Controllable*>& Connections() const { return _connections; }
	
	void StartIteration()
	{
		for(ControlValue& v : _values)
			v.Set(0);
		_nValuesSet = 0;
	}
	
	void SetNameGlobally(const std::string& effectName);
	
	std::unique_ptr<Effect> Copy() const;
	
protected:
	virtual void mix(const ControlValue* values, unsigned* channelValues, unsigned universe, const class Timing& timing) = 0;
	
	virtual std::string getControlName(size_t index) const = 0;
	
	void shallowAssign(const Effect& effect)
	{
		_values = effect._values;
		_nValuesSet = effect._nValuesSet;
		for(sigc::connection& c : _onDeleteConnections)
			c.disconnect();
		_controls.assign(effect._controls.size(), nullptr);
		_connections.clear();
		_onDeleteConnections.clear();
	}
private:
	friend class EffectControl;
	
	bool setControlValue(size_t index, const ControlValue& value)
	{
		_values[index] = value;
		++_nValuesSet;
		if(_nValuesSet == _values.size())
		{
			_nValuesSet = 0;
			return true;
		}
		else return false;
	}
	
	void mix(unsigned* channelValues, unsigned universe, const class Timing& timing)
	{
		mix(_values.data(), channelValues, universe, timing);
	}
	
	std::vector<ControlValue> _values;
	size_t _nValuesSet;
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
		_controls[i] = &control;
		control.attach(this, i);
		control.SetName(getControlName(i));
	}
	return controls;
}

#endif
