#ifndef GUI_STATE_H
#define GUI_STATE_H

#include <string>
#include <vector>
#include <memory>

#include <sigc++/signal.h>

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
	
	bool IsAssigned(const class PresetValue* p) const
	{
		for(const class PresetValue* preset : presets)
			if(p == preset)
				return true;
		return false;
	}
	
	// This list may contain nullptrs to indicate unset controls.
	std::vector<class PresetValue *> presets;
};

class GUIState
{
public:
	std::vector<std::unique_ptr<FaderSetupState>>& FaderSetups()
	{ return _faderSetups; }
	const std::vector<std::unique_ptr<FaderSetupState>>& FaderSetups() const
	{ return _faderSetups; }
	
	sigc::signal<void()>& FaderSetupSignalChange() { return _faderSetupSignalChange; }
	
	void EmitFaderSetupChangeSignal()
	{
		_faderSetupSignalChange();
	}
	
	void Clear()
	{
		_faderSetups.clear();
	}
	
	bool Empty() const { return _faderSetups.empty(); }
	
	bool IsAssigned(const class PresetValue* p) const
	{
		for(const std::unique_ptr<FaderSetupState>& fader : _faderSetups)
		{
			if(fader->IsAssigned(p))
				return true;
		}
		return false;
	}
	
private:
	sigc::signal<void()> _faderSetupSignalChange;
	std::vector<std::unique_ptr<FaderSetupState>> _faderSetups;
};

#endif
