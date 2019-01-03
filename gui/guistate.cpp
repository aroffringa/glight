#include "guistate.h"

#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"

void FaderSetupState::ChangeManagement(Management& management)
{
	for(PresetValue*& preset : presets)
	{
		if(preset != nullptr)
		{
			size_t presetId = preset->Id();
			preset = management.GetPresetValue(presetId);
		}
	}
}
