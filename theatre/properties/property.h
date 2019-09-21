#ifndef PROPERTY_H
#define PROPERTY_H

#include "../../theatre/folderobject.h"

#include "../../theatre/effects/thresholdeffect.h"

#include <string>

class Property final
{
public:
	enum Type {
		ControlValue,
		Duration,
		Boolean,
		Integer
	};
	Property(const std::string& name, const std::string& description, Type type) :
		_type(type),
		_setIndex(0),
		_name(name),
		_description(description)
	{ }
	
	Type GetType() const { return _type; }
	
	const std::string& Name() const { return _name; }
	
	const std::string& Description() const { return _description; }
	
private:
	friend class PropertySet;
	
	Type _type;
	size_t _setIndex;
	std::string _name, _description;
};

#endif
