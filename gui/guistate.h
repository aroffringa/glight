#ifndef GUI_STATE_H
#define GUI_STATE_H

#include <string>
#include <vector>
#include <memory>

#include <sigc++/connection.h>
#include <sigc++/signal.h>

class FaderState
{
public:
	explicit FaderState(class PresetValue* pv);
	FaderState() :
		_presetValue(nullptr),
		_isToggleButton(false),
		_newToggleButtonColumn(false)
	{ }
	FaderState(const FaderState& source);
	
	FaderState& operator=(const FaderState& rhs);
	
	~FaderState() { _presetValueDeletedConnection.disconnect(); }
	void SetPresetValue(class PresetValue* presetValue);
	void SetNoPresetValue() { SetPresetValue(nullptr); }
	void SetIsToggleButton(bool isToggle) { _isToggleButton = isToggle; }
	void SetNewToggleButtonColumn(bool newColumn) { _newToggleButtonColumn = newColumn; }
	
	// This might return a nullptr to indicate an unset control.
	class PresetValue* GetPresetValue() const { return _presetValue; }
	bool IsToggleButton() const { return _isToggleButton; }
	bool NewToggleButtonColumn() const { return _newToggleButtonColumn; }
	
private:
	void onPresetValueDeleted();
	class PresetValue* _presetValue;
	bool _isToggleButton;
	bool _newToggleButtonColumn;
	sigc::connection _presetValueDeletedConnection;
};

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
		for(const class FaderState& fader : faders)
			if(p == fader.GetPresetValue())
				return true;
		return false;
	}
	
	void ChangeManagement(class Management& management);
	
	std::vector<FaderState> faders;
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
	
	void ChangeManagement(class Management& management)
	{
		for(std::unique_ptr<FaderSetupState>& fader : _faderSetups)
			fader->ChangeManagement(management);
	}
	
private:
	sigc::signal<void()> _faderSetupSignalChange;
	std::vector<std::unique_ptr<FaderSetupState>> _faderSetups;
};

#endif
