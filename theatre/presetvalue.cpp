#include "presetvalue.h"

#include "controllable.h"

std::string PresetValue::Title() const
{
	if(_controllable->NInputs() > 1)
		return _controllable->Name() + " (" + 
			AbbreviatedFunctionType(_controllable->InputType(_inputIndex)) + ")";
	else
		return _controllable->Name();
}
