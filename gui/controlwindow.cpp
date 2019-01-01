#include "controlwindow.h"
#include "controlwidget.h"
#include "showwindow.h"

#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"
#include "../libtheatre/chase.h"

#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <glibmm/main.h>

const char ControlWindow::_keyRowsUpper[3][10] = {
	{ 'Z','X','C','V','B','N','M','<','>','?' },
	{ 'A','S','D','F','G','H','J','K','L',':' },
	{ 'Q','W','E','R','T','Y','U','I','O','P' } };
const char ControlWindow::_keyRowsLower[3][10] = {
	{ 'z','x','c','v','b','n','m',',','.','/' },
	{ 'a','s','d','f','g','h','j','k','l',';' },
	{ 'q','w','e','r','t','y','u','i','o','p' } };

ControlWindow::ControlWindow(class ShowWindow* showWindow, class Management &management, size_t keyRowIndex)
  : _management(management),
	_keyRowIndex(keyRowIndex),
	_faderSetupLabel("Fader setup: "),
	_nameButton("Name"),
	_newFaderSetupButton(Gtk::Stock::NEW),
	_soloCheckButton("Solo"),
	_fadeUpSpeed(0.0, 11.0, 1.0),
	_fadeDownSpeed(0.0, 11.0, 1.0),
	_addButton(Gtk::Stock::ADD),
	_assignButton("Assign"),
	_assignChasesButton("Chases"),
	_removeButton(Gtk::Stock::REMOVE),
	_showWindow(showWindow),
	_lastUpdateTime(boost::posix_time::microsec_clock::local_time())
{
	_showWindow->State().FaderSetups().emplace_back(new FaderSetupState());
	_state = _showWindow->State().FaderSetups().back().get();
	_state->name = "Unnamed fader setup";
	_state->isActive = true;
	_state->presets.assign(10, nullptr);
	
	initializeWidgets();
	
	_state->width = std::max(100, get_width());
	_state->height = std::max(300, get_height());
	AvoidRecursion::Token token(_delayUpdates);
	loadState();
}

ControlWindow::ControlWindow(class ShowWindow* showWindow, class Management &management, size_t keyRowIndex, FaderSetupState* state)
  : _management(management),
	_keyRowIndex(keyRowIndex),
	_faderSetupLabel("Fader setup: "),
	_nameButton("Name"),
	_newFaderSetupButton(Gtk::Stock::NEW),
	_soloCheckButton("Solo"),
	_fadeUpSpeed(0.0, 11.0, 1.0),
	_fadeDownSpeed(0.0, 11.0, 1.0),
	_addButton(Gtk::Stock::ADD),
	_assignButton("Assign"),
	_assignChasesButton("Chases"),
	_removeButton(Gtk::Stock::REMOVE),
	_showWindow(showWindow),
	_state(state),
	_lastUpdateTime(boost::posix_time::microsec_clock::local_time())
{
	_state->isActive = true;
	initializeWidgets();
	AvoidRecursion::Token token(_delayUpdates);
	loadState();
}

ControlWindow::~ControlWindow()
{
	_faderSetupChangeConnection.disconnect();
	_state->isActive = false;
	_showWindow->State().EmitFaderSetupChangeSignal();
}

void ControlWindow::initializeWidgets()
{
	set_title("Glight - controls");
	
	_faderSetupChangeConnection = _showWindow->State().FaderSetupSignalChange().connect(sigc::mem_fun(*this, &ControlWindow::updateFaderSetupList));
	
	signal_configure_event().connect(sigc::mem_fun(*this, &ControlWindow::onResize), false);
	
	_timeoutConnection = Glib::signal_timeout().connect( sigc::mem_fun(*this, &ControlWindow::onTimeout), 40);
	
	add(_vBox);
	
	_vBox.pack_start(_hBoxUpper, false, false);
	
	_hBoxUpper.pack_start(_faderSetupLabel, false, false);
	
	_faderSetupList = Gtk::ListStore::create(_faderSetupColumns);
	_faderSetup.set_model(_faderSetupList);
	_faderSetup.pack_start(_faderSetupColumns._name);
	updateFaderSetupList();
	_faderSetup.signal_changed().connect(sigc::mem_fun(*this, &ControlWindow::onFaderSetupChanged));
	
	_hBoxUpper.pack_start(_faderSetup, true, true);
	
	_nameButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onNameButtonClicked));
	_nameButton.set_image_from_icon_name("user-bookmarks");
	_hBoxUpper.pack_start(_nameButton, false, false, 5);

	_newFaderSetupButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onNewFaderSetupButtonClicked));
	_hBoxUpper.pack_start(_newFaderSetupButton, false, false, 5);

	_vBox.pack_start(_hBox2, true, true);
	_hBox2.pack_start(_buttonBox, false, false);
	
	_soloCheckButton.signal_toggled().connect(sigc::mem_fun(*this, &ControlWindow::onSoloButtonToggled));
	_buttonBox.pack_start(_soloCheckButton, false, false);
	
	_buttonBox.pack_start(_fadeUpSpeed, false, false);
	_fadeUpSpeed.signal_value_changed().connect(sigc::mem_fun(*this, &ControlWindow::onChangeUpSpeed));
	
	_buttonBox.pack_start(_fadeDownSpeed, false, false);
	_fadeDownSpeed.signal_value_changed().connect(sigc::mem_fun(*this, &ControlWindow::onChangeDownSpeed));

	_addButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onAddButtonClicked));
	_buttonBox.pack_start(_addButton, false, false);

	_assignButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onAssignButtonClicked));
	_buttonBox.pack_start(_assignButton, false, false);

	_assignChasesButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onAssignChasesButtonClicked));
	_buttonBox.pack_start(_assignChasesButton, false, false);

	_removeButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onRemoveButtonClicked));
	_buttonBox.pack_start(_removeButton, false, false);

	show_all_children();
	set_default_size(0, 300);
}

bool ControlWindow::onResize(GdkEventConfigure *event)
{
	if(_delayUpdates.IsFirst())
	{
		_state->height = get_height();
		_state->width = get_width();
	}
	return false;
}

void ControlWindow::UpdateAfterPresetRemoval()
{
	for(std::unique_ptr<ControlWidget>& cWidget : _controls)
	{
		cWidget->UpdateAfterPresetRemoval();
	}
}

void ControlWindow::addControl()
{
	if(_delayUpdates.IsFirst())
	{
		_state->presets.emplace_back(nullptr);
	}
	bool hasKey = _controls.size()<10 && _keyRowIndex<3;
	char key = hasKey ? _keyRowsLower[_keyRowIndex][_controls.size()] : ' ';
	std::unique_ptr<ControlWidget> control(new ControlWidget(_management, key));
	size_t controlIndex = _controls.size();
	control->SignalValueChange().connect(sigc::bind(sigc::mem_fun(*this, &ControlWindow::onControlValueChanged), control.get()));
	control->SignalValueChange().connect(sigc::bind(sigc::mem_fun(*this, &ControlWindow::onControlAssigned), controlIndex));
	_hBox2.pack_start(*control, true, false, 3);
	control->show();
	_controls.emplace_back(std::move(control));
}

void ControlWindow::onRemoveButtonClicked()
{
	if(_controls.size() > 1)
	{
		_controls.pop_back();
		_state->presets.pop_back();
	}
}

void ControlWindow::onAssignButtonClicked()
{
	size_t controlIndex = 0;
	for(std::unique_ptr<ControlWidget>& c : _controls)
		c->Unassign();
	size_t n = _management.PresetValues().size();
	if(!_controls.empty())
	{
		for(size_t i=0; i!=n; ++i)
		{
			PresetValue* p = _management.PresetValues()[i].get();
			if(!_showWindow->State().IsAssigned(p))
			{
				_controls[controlIndex]->Assign(p);
				++controlIndex;
				if(controlIndex == _controls.size())
					break;
			}
		}
	}
}

void ControlWindow::onAssignChasesButtonClicked()
{
	size_t controlIndex = 0;
	if(!_controls.empty())
	{
		for(size_t i=0; i!=_management.PresetValues().size(); ++i)
		{
			PresetValue* p = _management.PresetValues()[i].get();
			Chase* c = dynamic_cast<Chase*>(&p->Controllable());
			if(c != nullptr)
			{
				_controls[controlIndex]->Assign(p);
				++controlIndex;
				if(controlIndex == _controls.size())
					break;
			}
		}
	}
}

void ControlWindow::onSoloButtonToggled()
{ 
	if(_delayUpdates.IsFirst())
	{
		_state->isSolo = _soloCheckButton.get_active();
	}
}

void ControlWindow::onControlValueChanged(double newValue, ControlWidget* widget)
{
	if(_soloCheckButton.get_active())
	{
		// Limitting the controls might generate another control value change, but since
		// it is an auto generated change we will not apply the limit of that change to
		// other faders.
		if(_delayUpdates.IsFirst())
		{
			AvoidRecursion::Token token(_delayUpdates);
			double limitValue = ControlWidget::MAX_SCALE_VALUE() - newValue - ControlWidget::MAX_SCALE_VALUE()*0.01;
			if(limitValue < 0.0)
				limitValue = 0.0;
			for(std::unique_ptr<ControlWidget>& c : _controls)
			{
				if(c.get() != widget)
					c->Limit(limitValue);
			}
		}
	}
}

void ControlWindow::onControlAssigned(double newValue, size_t widgetIndex)
{
	if(_delayUpdates.IsFirst())
		_state->presets[widgetIndex] = _controls[widgetIndex]->Preset();
}

bool ControlWindow::HandleKeyDown(char key)
{
	if(_keyRowIndex >= 3) return false;
	
	for(unsigned i=0;i<10;++i) {
		if(_keyRowsUpper[_keyRowIndex][i] == key)
		{
			if(i < _controls.size())
			{
				_controls[i]->FullOn();
			}
			return true;
		}
		else if((_keyRowsLower[_keyRowIndex][i]) == key)
		{
			if(i < _controls.size())
			{
				_controls[i]->Toggle();
			}
			return true;
		}
	}
	return false;
}

bool ControlWindow::HandleKeyUp(char key)
{
	if(_keyRowIndex >= 3) return false;
	
	for(unsigned i=0;i<10;++i) {
		if(_keyRowsUpper[_keyRowIndex][i] == key)
		{
			if(i < _controls.size())
			{
				_controls[i]->FullOff();
			}
			return true;
		}
	}
	return false;
}

bool ControlWindow::IsAssigned(PresetValue* presetValue)
{
	for(std::unique_ptr<ControlWidget>& c : _controls)
	{
		if(c->Preset() == presetValue)
			return true;
	}
	return false;
}

void ControlWindow::onNameButtonClicked()
{
	Gtk::MessageDialog dialog(*this, "Name fader setup",
		false, Gtk::MESSAGE_QUESTION,
		Gtk::BUTTONS_OK_CANCEL);
	Gtk::Entry entry;
	dialog.get_vbox()->pack_start(entry, Gtk::PACK_SHRINK);
	dialog.get_vbox()->show_all_children();
  dialog.set_secondary_text(
		"Please enter a name for this fader setup");
	int result = dialog.run();
	if(result == Gtk::RESPONSE_OK)
	{
		_state->name = entry.get_text();
		_showWindow->State().EmitFaderSetupChangeSignal();
	}
}

void ControlWindow::onNewFaderSetupButtonClicked()
{
	AvoidRecursion::Token token(_delayUpdates);
	_state->isActive = false;
	_showWindow->State().FaderSetups().emplace_back(new FaderSetupState());
	_state = _showWindow->State().FaderSetups().back().get();
	_state->isActive = true;
	_state->name = "Unnamed fader setup";
	_state->presets.assign(10, nullptr);
	_state->height = 300;
	_state->width = 100;
	loadState();
	
	token.Release();
	
	updateFaderSetupList();
}

void ControlWindow::updateFaderSetupList()
{
	AvoidRecursion::Token token(_delayUpdates);
	GUIState& state = _showWindow->State();
	_faderSetupList->clear();
	for(const std::unique_ptr<FaderSetupState>& fState : state.FaderSetups())
	{
		bool itsMe = fState.get() == _state;
		if(!fState->isActive || itsMe)
		{
			Gtk::TreeModel::iterator row = _faderSetupList->append();
			(*row)[_faderSetupColumns._obj] = fState.get();
			(*row)[_faderSetupColumns._name] = fState->name;
			if(itsMe)
			{
				_faderSetup.set_active(row);
			}
		}
	}
}

void ControlWindow::onFaderSetupChanged()
{
	if(_delayUpdates.IsFirst())
	{
		AvoidRecursion::Token token(_delayUpdates);
		_state->isActive = false;
		_state->height = get_height();
		_state->width = get_width();
		_state = (*_faderSetup.get_active())[_faderSetupColumns._obj];
		_state->isActive = true;
		
		loadState();
		
		token.Release();
		
		_showWindow->State().EmitFaderSetupChangeSignal();
	}
}

void ControlWindow::loadState()
{
	_soloCheckButton.set_active(_state->isSolo);
	
	while(_controls.size() < _state->presets.size())
		addControl();
	_controls.resize(_state->presets.size()); // remove controls if there were too many
	
	resize(_state->width, _state->height);
	
	for(size_t i=0; i!=_state->presets.size(); ++i)
		_controls[i]->Assign(_state->presets[i]);
}

void ControlWindow::updateValues()
{
	boost::posix_time::ptime currentTime(boost::posix_time::microsec_clock::local_time());
	double timePassed = (double) (currentTime - _lastUpdateTime).total_microseconds() * 1e-6;
	_lastUpdateTime = std::move(currentTime);
	for(std::unique_ptr<ControlWidget>& cw : _controls)
	{
		cw->UpdateValue(timePassed);
	}
}

double ControlWindow::mapSliderToSpeed(int sliderVal)
{
	switch(sliderVal)
	{
		default:
		case 0: return 0.0;
		case 1: return 10.0;
		case 2: return 5.0;
		case 3: return 3.5;
		case 4: return 2.0;
		case 5: return 1.0;
		case 6: return 0.5;
		case 7: return 0.33;
		case 8: return 0.25;
		case 9: return 0.16;
		case 10: return 0.1;
	}
}

void ControlWindow::onChangeDownSpeed()
{
	double speed = mapSliderToSpeed(int(_fadeDownSpeed.get_value()));

	for(std::unique_ptr<ControlWidget>& cw : _controls)
		cw->SetFadeDownSpeed(speed);
}

void ControlWindow::onChangeUpSpeed()
{
	double speed = mapSliderToSpeed(int(_fadeUpSpeed.get_value()));
	
	for(std::unique_ptr<ControlWidget>& cw : _controls)
		cw->SetFadeUpSpeed(speed);
}
