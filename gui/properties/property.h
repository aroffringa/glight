#ifndef PROPERTY_H
#define PROPERTY_H

#include "../../libtheatre/namedobject.h"

#include "../../libtheatre/effects/thresholdeffect.h"

#include <string>

class Property final
{
public:
	enum Type {
		ControlValue
	};
	Property(const std::string& title, Type type) :
		_type(type),
		_setIndex(0),
		_title(title)
	{ }
	
private:
	friend class PropertySet;
	
	Type _type;
	size_t _setIndex;
	std::string _title;
};

#endif
