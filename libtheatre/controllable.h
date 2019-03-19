#ifndef CONTROL_H
#define CONTROL_H

#include <string>

#include "controlvalue.h"
#include "namedobject.h"

/**
	@author Andre Offringa
*/
class Controllable : public NamedObject {
	public:
		Controllable() { }
		Controllable(const Controllable& source) : NamedObject(source) { }
		Controllable(const std::string &name) : NamedObject(name) { }
		
		virtual void Mix(const ControlValue& value, unsigned* channelValues, unsigned universe, const class Timing& timing) = 0;
};

#endif
