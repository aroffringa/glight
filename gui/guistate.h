#ifndef GUI_STATE_H
#define GUI_STATE_H

#include <string>
#include <vector>
#include <memory>

struct FaderSetupState
{
	std::string name;
	bool isActive;
	
	// This list may contain nullptrs to indicate unset controls.
	std::vector<class PresetValue *> _presets;
};

struct GUIState
{
	std::vector<std::unique_ptr<FaderSetupState>> _faderSetups;
};

#endif
