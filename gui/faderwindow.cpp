#include "faderwindow.h"
#include "faderwidget.h"
#include "showwindow.h"

#include "../theatre/management.h"
#include "../theatre/presetvalue.h"
#include "../theatre/chase.h"

#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <glibmm/main.h>

const char FaderWindow::_keyRowsUpper[3][10] = {
	{ 'Z','X','C','V','B','N','M','<','>','?' },
	{ 'A','S','D','F','G','H','J','K','L',':' },
	{ 'Q','W','E','R','T','Y','U','I','O','P' } };
const char FaderWindow::_keyRowsLower[3][10] = {
	{ 'z','x','c','v','b','n','m',',','.','/' },
	{ 'a','s','d','f','g','h','j','k','l',';' },
	{ 'q','w','e','r','t','y','u','i','o','p' } };

FaderWindow::FaderWindow(class ShowWindow* showWindow, class Management &management, size_t keyRowIndex)
  : _management(&management),
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
	for(size_t i=0; i!=10; ++i)
		_state->faders.emplace_back();
	
	initializeWidgets();
	
	_state->width = std::max(100, get_width());
	_state->height = std::max(300, get_height());
	RecursionLock::Token token(_delayUpdates);
	loadState();
}

FaderWindow::FaderWindow(class ShowWindow* showWindow, class Management &management, size_t keyRowIndex, FaderSetupState* state)
  : _management(&management),
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
	RecursionLock::Token token(_delayUpdates);
	loadState();
}

FaderWindow::~FaderWindow()
{
	_faderSetupChangeConnection.disconnect();
	_state->isActive = false;
	_showWindow->State().EmitFaderSetupChangeSignal();
}

void FaderWindow::initializeWidgets()
{
	set_title("Glight - controls");
	
	_faderSetupChangeConnection = _showWindow->State().FaderSetupSignalChange().connect(sigc::mem_fun(*this, &FaderWindow::updateFaderSetupList));
	
	signal_configure_event().connect(sigc::mem_fun(*this, &FaderWindow::onResize), false);
	
	_timeoutConnection = Glib::signal_timeout().connect( sigc::mem_fun(*this, &FaderWindow::onTimeout), 40);
	
	add(_vBox);
	
	_vBox.pack_start(_hBoxUpper, false, false);
	
	_hBoxUpper.pack_start(_faderSetupLabel, false, false);
	
	_faderSetupList = Gtk::ListStore::create(_faderSetupColumns);
	_faderSetup.set_model(_faderSetupList);
	_faderSetup.pack_start(_faderSetupColumns._name);
	updateFaderSetupList();
	_faderSetup.signal_changed().connect(sigc::mem_fun(*this, &FaderWindow::onFaderSetupChanged));
	
	_hBoxUpper.pack_start(_faderSetup, true, true);
	
	_nameButton.signal_clicked().connect(sigc::mem_fun(*this, &FaderWindow::onNameButtonClicked));
	_nameButton.set_image_from_icon_name("user-bookmarks");
	_hBoxUpper.pack_start(_nameButton, false, false, 5);

	_newFaderSetupButton.signal_clicked().connect(sigc::mem_fun(*this, &FaderWindow::onNewFaderSetupButtonClicked));
	_hBoxUpper.pack_start(_newFaderSetupButton, false, false, 5);

	_vBox.pack_start(_hBox2, true, true);
	_hBox2.pack_start(_buttonBox, false, false);
	_controlGrid.set_column_spacing(3);
	_hBox2.pack_start(_controlGrid, true, true);
	
	_soloCheckButton.signal_toggled().connect(sigc::mem_fun(*this, &FaderWindow::onSoloButtonToggled));
	_buttonBox.pack_start(_soloCheckButton, false, false);
	
	_buttonBox.pack_start(_fadeUpSpeed, false, false);
	_fadeUpSpeed.signal_value_changed().connect(sigc::mem_fun(*this, &FaderWindow::onChangeUpSpeed));
	
	_buttonBox.pack_start(_fadeDownSpeed, false, false);
	_fadeDownSpeed.signal_value_changed().connect(sigc::mem_fun(*this, &FaderWindow::onChangeDownSpeed));

	_addButton.signal_clicked().connect(sigc::mem_fun(*this, &FaderWindow::onAddButtonClicked));
	_buttonBox.pack_start(_addButton, false, false);

	_assignButton.signal_clicked().connect(sigc::mem_fun(*this, &FaderWindow::onAssignButtonClicked));
	_buttonBox.pack_start(_assignButton, false, false);

	_assignChasesButton.signal_clicked().connect(sigc::mem_fun(*this, &FaderWindow::onAssignChasesButtonClicked));
	_buttonBox.pack_start(_assignChasesButton, false, false);

	_removeButton.signal_clicked().connect(sigc::mem_fun(*this, &FaderWindow::onRemoveButtonClicked));
	_buttonBox.pack_start(_removeButton, false, false);

	show_all_children();
	set_default_size(0, 300);
}

bool FaderWindow::onResize(GdkEventConfigure *event)
{
	if(_delayUpdates.IsFirst())
	{
		_state->height = get_height();
		_state->width = get_width();
	}
	return false;
}

void FaderWindow::Update()
{
	for(std::unique_ptr<ControlWidget>& cWidget : _controls)
	{
		cWidget->Update();
	}
}

void FaderWindow::addControl()
{
	if(_delayUpdates.IsFirst())
	{
		_state->faders.emplace_back();
	}
	bool hasKey = _controls.size()<10 && _keyRowIndex<3;
	char key = hasKey ? _keyRowsLower[_keyRowIndex][_controls.size()] : ' ';
	std::unique_ptr<ControlWidget> control(new ControlWidget(*_management, *_showWindow, key));
	control->SetFadeDownSpeed(mapSliderToSpeed(int(_fadeDownSpeed.get_value())));
	control->SetFadeUpSpeed(mapSliderToSpeed(int(_fadeUpSpeed.get_value())));
	size_t controlIndex = _controls.size();
	control->SignalValueChange().connect(sigc::bind(sigc::mem_fun(*this, &FaderWindow::onControlValueChanged), control.get()));
	control->SignalValueChange().connect(sigc::bind(sigc::mem_fun(*this, &FaderWindow::onControlAssigned), controlIndex));
	
	_controlGrid.attach(*control, _controls.size()*2+1, 0, 2, 1);
	control->NameLabel().set_hexpand(true);
	bool even = _controls.size()%2==0;
	_controlGrid.attach(control->NameLabel(), _controls.size()*2, even ? 1 : 2, 4, 1);
	
	control->show();
	_controls.emplace_back(std::move(control));
}

void FaderWindow::onRemoveButtonClicked()
{
	if(_controls.size() > 1)
	{
		_controls.pop_back();
		_state->faders.pop_back();
	}
}

void FaderWindow::onAssignButtonClicked()
{
	size_t controlIndex = 0;
	for(std::unique_ptr<ControlWidget>& c : _controls)
		c->Unassign();
	size_t n = _management->PresetValues().size();
	if(!_controls.empty())
	{
		for(size_t i=0; i!=n; ++i)
		{
			PresetValue* p = _management->PresetValues()[i].get();
			if(!_showWindow->State().IsAssigned(p))
			{
				_controls[controlIndex]->Assign(p, true);
				++controlIndex;
				if(controlIndex == _controls.size())
					break;
			}
		}
	}
}

void FaderWindow::onAssignChasesButtonClicked()
{
	size_t controlIndex = 0;
	if(!_controls.empty())
	{
		for(size_t i=0; i!=_management->PresetValues().size(); ++i)
		{
			PresetValue* p = _management->PresetValues()[i].get();
			Chase* c = dynamic_cast<Chase*>(&p->Controllable());
			if(c != nullptr)
			{
				_controls[controlIndex]->Assign(p, true);
				++controlIndex;
				if(controlIndex == _controls.size())
					break;
			}
		}
	}
}

void FaderWindow::onSoloButtonToggled()
{ 
	if(_delayUpdates.IsFirst())
	{
		_state->isSolo = _soloCheckButton.get_active();
	}
}

void FaderWindow::onControlValueChanged(double newValue, ControlWidget* widget)
{
	if(_soloCheckButton.get_active())
	{
		// Limitting the controls might generate another control value change, but since
		// it is an auto generated change we will not apply the limit of that change to
		// other faders.
		if(_delayUpdates.IsFirst())
		{
			RecursionLock::Token token(_delayUpdates);
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

void FaderWindow::onControlAssigned(double newValue, size_t widgetIndex)
{
	if(_delayUpdates.IsFirst())
		_state->faders[widgetIndex].SetPresetValue(_controls[widgetIndex]->Preset());
}

bool FaderWindow::HandleKeyDown(char key)
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

bool FaderWindow::HandleKeyUp(char key)
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

bool FaderWindow::IsAssigned(PresetValue* presetValue)
{
	for(std::unique_ptr<ControlWidget>& c : _controls)
	{
		if(c->Preset() == presetValue)
			return true;
	}
	return false;
}

void FaderWindow::onNameButtonClicked()
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

void FaderWindow::onNewFaderSetupButtonClicked()
{
	RecursionLock::Token token(_delayUpdates);
	_state->isActive = false;
	_showWindow->State().FaderSetups().emplace_back(new FaderSetupState());
	_state = _showWindow->State().FaderSetups().back().get();
	_state->isActive = true;
	_state->name = "Unnamed fader setup";
	for(size_t i=0; i!=10; ++i)
		_state->faders.emplace_back();
	_state->height = 300;
	_state->width = 100;
	loadState();
	
	token.Release();
	
	updateFaderSetupList();
}

void FaderWindow::updateFaderSetupList()
{
	RecursionLock::Token token(_delayUpdates);
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

void FaderWindow::onFaderSetupChanged()
{
	if(_delayUpdates.IsFirst())
	{
		RecursionLock::Token token(_delayUpdates);
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

void FaderWindow::loadState()
{
	_soloCheckButton.set_active(_state->isSolo);
	
	while(_controls.size() < _state->faders.size())
		addControl();
	_controls.resize(_state->faders.size()); // remove controls if there were too many
	
	resize(_state->width, _state->height);
	
	for(size_t i=0; i!=_state->faders.size(); ++i)
		_controls[i]->Assign(_state->faders[i].GetPresetValue(), true);
}

void FaderWindow::updateValues()
{
	boost::posix_time::ptime currentTime(boost::posix_time::microsec_clock::local_time());
	double timePassed = (double) (currentTime - _lastUpdateTime).total_microseconds() * 1e-6;
	_lastUpdateTime = std::move(currentTime);
	for(std::unique_ptr<ControlWidget>& cw : _controls)
	{
		cw->UpdateValue(timePassed);
	}
}

double FaderWindow::mapSliderToSpeed(int sliderVal)
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

void FaderWindow::onChangeDownSpeed()
{
	double speed = mapSliderToSpeed(int(_fadeDownSpeed.get_value()));

	for(std::unique_ptr<ControlWidget>& cw : _controls)
		cw->SetFadeDownSpeed(speed);
}

void FaderWindow::onChangeUpSpeed()
{
	double speed = mapSliderToSpeed(int(_fadeUpSpeed.get_value()));
	
	for(std::unique_ptr<ControlWidget>& cw : _controls)
		cw->SetFadeUpSpeed(speed);
}

void FaderWindow::ChangeManagement(class Management& management, bool moveSliders)
{
	_management = &management;
	for(std::unique_ptr<ControlWidget>& cw : _controls)
	{
		cw->ChangeManagement(management, moveSliders);
	}
}
