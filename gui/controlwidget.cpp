#include "controlwidget.h"

#include "dialogs/inputselectdialog.h"

#include "../theatre/presetvalue.h"
#include "../theatre/management.h"
#include "../theatre/controllable.h"

#define MAX_SCALE_VALUE_DEF (1<<24)

ControlWidget::ControlWidget(class Management &management, ShowWindow& showWindow, char key) :
  _scale(0, MAX_SCALE_VALUE_DEF, MAX_SCALE_VALUE_DEF/100),
	_flashButton(std::string(1, key)),
	_nameLabel("<..>"),
	_management(&management),
	_showWindow(showWindow),
	_preset(nullptr),
	_fadeUpSpeed(0.0),
	_fadeDownSpeed(0.0),
	_fadingValue(0),
	_targetValue(0),
	_holdUpdates(false)
{
	_scale.set_inverted(true);
	_scale.set_draw_value(false);
	_scale.set_vexpand(true);
	_scale.signal_value_changed().
		connect(sigc::mem_fun(*this, &ControlWidget::onScaleChange));
	pack_start(_scale, true, true, 0);
	_scale.show();

	_flashButton.set_events(Gdk::BUTTON_PRESS_MASK);
	_flashButton.signal_button_press_event().connect(sigc::mem_fun(*this, &ControlWidget::onFlashButtonPressed), false);
	_flashButton.set_events(Gdk::BUTTON_PRESS_MASK);
	_flashButton.signal_button_release_event().connect(sigc::mem_fun(*this, &ControlWidget::onFlashButtonReleased), false);
	pack_start(_flashButton, false, false, 0);
	_flashButton.show();

	_onCheckButton.set_halign(Gtk::ALIGN_CENTER);
	_onCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ControlWidget::onOnButtonClicked));
	pack_start(_onCheckButton, false, false, 0);
	_onCheckButton.show();

	//pack_start(_eventBox, false, false, 0);
	_eventBox.set_events(Gdk::BUTTON_PRESS_MASK);
	_eventBox.show();

	_eventBox.signal_button_press_event().
		connect(sigc::mem_fun(*this, &ControlWidget::onNameLabelClicked));
	_eventBox.add(_nameLabel);
	_nameLabel.show();
}

ControlWidget::~ControlWidget()
{ }

double ControlWidget::MAX_SCALE_VALUE()
{
	return MAX_SCALE_VALUE_DEF;
}

void ControlWidget::onOnButtonClicked()
{
	if(!_holdUpdates)
	{
		_holdUpdates = true;
		if(_onCheckButton.get_active())
			_scale.set_value(MAX_SCALE_VALUE_DEF-1);
		else
			_scale.set_value(0);
		_holdUpdates = false;

		writeValue();
		_signalValueChange.emit(_scale.get_value());
	}
}

bool ControlWidget::onFlashButtonPressed(GdkEventButton* event)
{
	_scale.set_value(MAX_SCALE_VALUE_DEF-1);
	return false;
}

bool ControlWidget::onFlashButtonReleased(GdkEventButton* event)
{
	_scale.set_value(0);
	return false;
}

void ControlWidget::onScaleChange()
{
	if(!_holdUpdates)
	{
		_holdUpdates = true;
		_onCheckButton.set_active(_scale.get_value() != 0);
		_holdUpdates = false;

		writeValue();
		_signalValueChange.emit(_scale.get_value());
	}
}

bool ControlWidget::onNameLabelClicked(GdkEventButton* event)
{
	InputSelectDialog dialog(*_management, _showWindow);
	if(dialog.run() == Gtk::RESPONSE_OK)
	{
		Assign(dialog.SelectedInputPreset(), true);
	}
	return true;
}

void ControlWidget::writeValue()
{
	_targetValue = _scale.get_value();
	double fadeSpeed =
		(_targetValue > _fadingValue) ? _fadeUpSpeed : _fadeDownSpeed;
	if(fadeSpeed == 0.0)
	{
		_fadingValue = _targetValue;
		if(_preset != nullptr)
		{
			_preset->Value().Set(_targetValue);
		}
	}
}

void ControlWidget::Assign(PresetValue* item, bool moveFader)
{
	if(item != _preset)
	{
		_preset = item;
		if(_preset != nullptr)
		{
			_nameLabel.set_text(_preset->Title());
			if(moveFader)
			{
				_fadingValue = _preset->Value().UInt();
				_scale.set_value(_preset->Value().UInt());
			}
			else
				writeValue();
		}
		else {
			_nameLabel.set_text("<..>");
			if(moveFader)
			{
				_fadingValue = 0;
				_scale.set_value(0);
			}
			else
				writeValue();
		}
		_signalAssigned();
		if(moveFader)
			_signalValueChange.emit(_scale.get_value());
	}
}

void ControlWidget::Update()
{
	if(_preset != nullptr)
	{
		// The preset might be removed, if so update label
		if(!_management->Contains(*_preset))
		{
			_nameLabel.set_text("<..>");
			_preset = nullptr;
			_scale.set_value(0.0);
		}
		// Only if not removed: if preset is renamed, update
		else {
			_nameLabel.set_text(_preset->Title());
		}
	}
}

void ControlWidget::Toggle()
{
	_onCheckButton.set_active(!_onCheckButton.get_active());
}

void ControlWidget::FullOn()
{
	_scale.set_value(MAX_SCALE_VALUE_DEF-1);
}

void ControlWidget::FullOff()
{
	_scale.set_value(0);
}

void ControlWidget::UpdateValue(double timePassed)
{
	if(_targetValue != _fadingValue)
	{
		_targetValue = _scale.get_value();
		double fadeSpeed =
			(_targetValue > _fadingValue) ? _fadeUpSpeed : _fadeDownSpeed;
		if(fadeSpeed == 0.0)
		{
			_fadingValue = _targetValue;
		}
		else {
			unsigned stepSize = unsigned(std::min<double>(timePassed * fadeSpeed * double(MAX_SCALE_VALUE_DEF), double(MAX_SCALE_VALUE_DEF)));
			if(_targetValue > _fadingValue)
			{
				if(_fadingValue + stepSize > _targetValue)
					_fadingValue = _targetValue;
				else
					_fadingValue += stepSize;
			}
			else {
				if(_targetValue + stepSize > _fadingValue)
					_fadingValue = _targetValue;
				else
					_fadingValue -= stepSize;
			}
		}
		if(_preset != nullptr)
		{
			_preset->Value().Set(_fadingValue);
		}
	}
}

void ControlWidget::ChangeManagement(class Management& management, bool moveSliders)
{
	if(_preset == nullptr)
	{
		_management = &management;
	}
	else {
		std::string controllablePath = _preset->Controllable().FullPath();
		size_t input = _preset->InputIndex();
		_management = &management;
		Controllable& controllable = static_cast<Controllable&>(_management->GetObjectFromPath(controllablePath));
		PresetValue* pv = _management->GetPresetValue(controllable, input);
		if(pv == nullptr)
			Unassign();
		else {
			Assign(pv, moveSliders);
		}
	}
}
