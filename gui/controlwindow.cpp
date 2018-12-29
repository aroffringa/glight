#include "controlwindow.h"
#include "controlwidget.h"
#include "showwindow.h"

#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"
#include "../libtheatre/chase.h"

#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

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
	_soloCheckButton("Solo"),
	_addButton(Gtk::Stock::ADD),
	_assignButton("Assign"),
	_assignChasesButton("Chases"),
	_removeButton(Gtk::Stock::REMOVE),
	_showWindow(showWindow)
{
	set_title("Glight - controls");
	
	_showWindow->State()._faderSetups.emplace_back(new FaderSetupState());
	_state = _showWindow->State()._faderSetups.back().get();
	_state->name = "Unnamed fader setup";
	_state->isActive = true;

	add(_vBox);
	
	_vBox.pack_start(_hBoxUpper, false, false);
	
	_hBoxUpper.pack_start(_faderSetupLabel, false, false);
	
	_faderSetupList = Gtk::ListStore::create(_faderSetupColumns);
	_faderSetup.set_model(_faderSetupList);
	_faderSetup.pack_start(_faderSetupColumns._name);
	updateFaderSetupList();
	
	_hBoxUpper.pack_start(_faderSetup, true, true);
	
	   _nameButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onNameButtonClicked));
	   _nameButton.set_image_from_icon_name("user-bookmarks");
	_hBoxUpper.pack_start(_nameButton, false, false, 5);

	_vBox.pack_start(_hBox2, true, true);
	_hBox2.pack_start(_buttonBox, false, false);

	for(int i=0;i<10;++i)
	{
		addControl();
	}
	
	_soloCheckButton.signal_toggled().connect(sigc::mem_fun(*this, &ControlWindow::onSoloButtonToggled));
	_buttonBox.pack_start(_soloCheckButton, false, false);

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

ControlWindow::~ControlWindow()
{
	_state->isActive = false;
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
	bool hasKey = _controls.size()<10 && _keyRowIndex<3;
	char key = hasKey ? _keyRowsLower[_keyRowIndex][_controls.size()] : ' ';
	std::unique_ptr<ControlWidget> control(new ControlWidget(_management, key));
	control->SignalChange().connect(sigc::bind(sigc::mem_fun(*this, &ControlWindow::onControlValueChanged), control.get()));
	_hBox2.pack_start(*control, true, true, 3);
	control->show();
	_controls.emplace_back(std::move(control));
}

void ControlWindow::onRemoveButtonClicked()
{
	if(_controls.size() > 1)
	{
		_controls.pop_back();
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
			if(!_showWindow->IsAssignedToControl(p))
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
{ }

void ControlWindow::onControlValueChanged(double newValue, ControlWidget* widget)
{
	if(_soloCheckButton.get_active())
	{
		// Limitting the controls might generate another control value change, but since
		// it is an auto generated change we will not apply the limit of that change to
		// other faders.
		static bool inEvent = false;
		if(!inEvent)
		{
			inEvent = true;
			double limitValue = ControlWidget::MAX_SCALE_VALUE() - newValue - ControlWidget::MAX_SCALE_VALUE()*0.01;
			if(limitValue < 0.0)
				limitValue = 0.0;
			for(std::unique_ptr<ControlWidget>& c : _controls)
			{
				if(c.get() != widget)
					c->Limit(limitValue);
			}
			inEvent = false;
		}
	}
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
		updateFaderSetupList();
	}
}

void ControlWindow::updateFaderSetupList()
{
	GUIState& state = _showWindow->State();
	_faderSetupList->clear();
	for(std::unique_ptr<FaderSetupState>& fState : state._faderSetups)
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
