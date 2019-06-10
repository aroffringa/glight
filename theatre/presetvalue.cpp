#include "presetvalue.h"

#include "controllable.h"

std::string PresetValue::Title() const
{
	if(_controllable->NInputs() > 1)
		return _controllable->Name() + " (" + _controllable->InputName(_inputIndex) + ")";
	else
		return _controllable->Name();
}
