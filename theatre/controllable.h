#ifndef CONTROL_H
#define CONTROL_H

#include <string>

#include "controlvalue.h"
#include "folderobject.h"

/**
	@author Andre Offringa
*/
class Controllable : public FolderObject {
public:
	Controllable() : _visitLevel(0) { }
	Controllable(const Controllable& source) : FolderObject(source), _visitLevel(0) { }
	Controllable(const std::string& name) : FolderObject(name), _visitLevel(0) { }
	
	virtual size_t NInputs() const = 0;
	
	virtual ControlValue& InputValue(size_t index) = 0;
	
	virtual std::string InputName(size_t index) = 0;
	
	void MixInput(size_t index, const ControlValue& value)
	{
		unsigned mixVal = ControlValue::Mix(InputValue(index).UInt(), value.UInt(), ControlValue::Default);
		InputValue(index) = ControlValue(mixVal);
	}
	
	virtual size_t NOutputs() const = 0;
	
	virtual std::pair<Controllable*, size_t> Output(size_t index) const = 0;
	
	virtual void Mix(unsigned* channelValues, unsigned universe, const class Timing& timing) = 0;
	
	bool HasOutputConnection(const Controllable& controllable) const
	{
		for(size_t i=0; i!=NOutputs(); ++i)
			if(Output(i).first == &controllable)
				return true;
		return false;
	}
	
	/* Used for dependency analysis. */
	char VisitLevel() const { return _visitLevel; }
	
	void SetVisitLevel(char visitLevel) { _visitLevel = visitLevel; }
	
private:
	ControlValue _inputValue;
	char _visitLevel;
};

#endif