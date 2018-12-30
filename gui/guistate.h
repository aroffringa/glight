#ifndef GUI_STATE_H
#define GUI_STATE_H

#include <string>
#include <vector>
#include <memory>

class FaderSetupState
{
public:
	FaderSetupState() :
		isActive(false), isSolo(false),
		width(0), height(0)
	{ }
	std::string name;
	bool isActive;
	bool isSolo;
	size_t width, height;
	
	// This list may contain nullptrs to indicate unset controls.
	std::vector<class PresetValue *> presets;
};

class GUIState
{
public:
	std::vector<std::unique_ptr<FaderSetupState>>& FaderSetups()
	{ return _faderSetups; }
	
	sigc::signal<void()>& FaderSetupSignalChange() { return _faderSetupSignalChange; }
	
	void EmitFaderSetupChangeSignal()
	{
		_faderSetupSignalChange();
	}
	
private:
	sigc::signal<void()> _faderSetupSignalChange;
	std::vector<std::unique_ptr<FaderSetupState>> _faderSetups;
};

#endif
