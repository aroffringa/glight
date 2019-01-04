#include "guistate.h"

#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"

FaderState::FaderState(class PresetValue* presetValue) :
	_presetValue(presetValue)
{
	if(presetValue != nullptr)
		_presetValueDeletedConnection = presetValue->SignalDelete().connect([&](){ onPresetValueDeleted(); } );
}

FaderState::FaderState(const FaderState& source) :
	_presetValue(source._presetValue)
{
	if(_presetValue != nullptr)
		_presetValueDeletedConnection = _presetValue->SignalDelete().connect([&](){ onPresetValueDeleted(); } );
}

FaderState& FaderState::operator=(const FaderState& rhs)
{
	_presetValueDeletedConnection.disconnect();
	_presetValue = rhs._presetValue;
	if(_presetValue != nullptr)
		_presetValueDeletedConnection = _presetValue->SignalDelete().connect([&](){ onPresetValueDeleted(); } );
	return *this;
}

void FaderState::SetPresetValue(PresetValue* presetValue)
{
	_presetValueDeletedConnection.disconnect();
	_presetValue = presetValue;
	if(presetValue != nullptr)
		_presetValueDeletedConnection = presetValue->SignalDelete().connect([&](){ onPresetValueDeleted(); } );
}

void FaderState::onPresetValueDeleted()
{
	_presetValueDeletedConnection.disconnect();
	_presetValue = nullptr;
}

void FaderSetupState::ChangeManagement(Management& management)
{
	for(FaderState& fader : faders)
	{
		if(fader.GetPresetValue() != nullptr)
		{
			size_t presetId = fader.GetPresetValue()->Id();
			fader.SetPresetValue(management.GetPresetValue(presetId));
		}
	}
}
