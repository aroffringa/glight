#include "controlwindow.h"
#include "controlwidget.h"
#include "showwindow.h"

#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"
#include "../libtheatre/chase.h"

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
  _soloCheckButton("Solo"),
	_addButton(Gtk::Stock::ADD),
	_assignButton("Assign"),
	_assignChasesButton("Chases"),
	_removeButton(Gtk::Stock::REMOVE),
	_showWindow(showWindow)
{
	set_title("Glight - controls");

	add(_hBox);
	_hBox.show();

	_hBox.pack_start(_buttonBox, false, false, 0);

	for(int i=0;i<10;++i)
	{
		addControl();
	}
	
	_soloCheckButton.signal_toggled().connect(sigc::mem_fun(*this, &ControlWindow::onSoloButtonToggled));
	_buttonBox.pack_start(_soloCheckButton, false, false, 0);

	_addButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onAddButtonClicked));
	_buttonBox.pack_start(_addButton, false, false, 0);

	_assignButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onAssignButtonClicked));
	_buttonBox.pack_start(_assignButton, false, false, 0);

	_assignChasesButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onAssignChasesButtonClicked));
	_buttonBox.pack_start(_assignChasesButton, false, false, 0);

	_removeButton.signal_clicked().connect(sigc::mem_fun(*this, &ControlWindow::onRemoveButtonClicked));
	_buttonBox.pack_start(_removeButton, false, false, 0);

	show_all_children();
	set_default_size(0, 300);
}

ControlWindow::~ControlWindow()
{
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
	_hBox.pack_start(*control, true, true, 3);
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
