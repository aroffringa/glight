#include "faderwidget.h"

#include "../eventtransmitter.h"

#include "../dialogs/inputselectdialog.h"

#include "../../theatre/presetvalue.h"
#include "../../theatre/management.h"
#include "../../theatre/controllable.h"

FaderWidget::FaderWidget(class Management &management, EventTransmitter& eventHub, char key) :
  _scale(0, ControlValue::MaxUInt()+1, (ControlValue::MaxUInt()+1)/100),
	_flashButton(std::string(1, key)),
	_nameLabel("<..>"),
	_management(&management),
	_eventHub(eventHub),
	_preset(nullptr),
	_holdUpdates(false)
{
	_updateConnection = _eventHub.SignalUpdateControllables().connect( [&](){ onUpdate(); } );
	
	_scale.set_inverted(true);
	_scale.set_draw_value(false);
	_scale.set_vexpand(true);
	_scale.signal_value_changed().
		connect(sigc::mem_fun(*this, &FaderWidget::onScaleChange));
	_box.pack_start(_scale, true, true, 0);
	_scale.show();

	_flashButton.set_events(Gdk::BUTTON_PRESS_MASK);
	_flashButton.signal_button_press_event().connect(sigc::mem_fun(*this, &FaderWidget::onFlashButtonPressed), false);
	_flashButton.set_events(Gdk::BUTTON_PRESS_MASK);
	_flashButton.signal_button_release_event().connect(sigc::mem_fun(*this, &FaderWidget::onFlashButtonReleased), false);
	_box.pack_start(_flashButton, false, false, 0);
	_flashButton.show();

	_onCheckButton.set_halign(Gtk::ALIGN_CENTER);
	_onCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &FaderWidget::onOnButtonClicked));
	_box.pack_start(_onCheckButton, false, false, 0);
	_onCheckButton.show();

	//pack_start(_eventBox, false, false, 0);
	_eventBox.set_events(Gdk::BUTTON_PRESS_MASK);
	_eventBox.show();

	_eventBox.signal_button_press_event().
		connect(sigc::mem_fun(*this, &FaderWidget::onNameLabelClicked));
	_eventBox.add(_nameLabel);
	_nameLabel.show();
	
	add(_box);
	_box.show();
}

FaderWidget::~FaderWidget()
{
	_updateConnection.disconnect();
}

void FaderWidget::onOnButtonClicked()
{
	if(!_holdUpdates)
	{
		_holdUpdates = true;
		if(_onCheckButton.get_active())
			_scale.set_value(ControlValue::MaxUInt());
		else
			_scale.set_value(0);
		_holdUpdates = false;

		writeValue(_scale.get_value());
		SignalValueChange().emit(_scale.get_value());
	}
}

bool FaderWidget::onFlashButtonPressed(GdkEventButton* event)
{
	_scale.set_value(ControlValue::MaxUInt());
	return false;
}

bool FaderWidget::onFlashButtonReleased(GdkEventButton* event)
{
	_scale.set_value(0);
	return false;
}

void FaderWidget::onScaleChange()
{
	if(!_holdUpdates)
	{
		_holdUpdates = true;
		_onCheckButton.set_active(_scale.get_value() != 0);
		_holdUpdates = false;

		writeValue(_scale.get_value());
		SignalValueChange().emit(_scale.get_value());
	}
}

bool FaderWidget::onNameLabelClicked(GdkEventButton* event)
{
	InputSelectDialog dialog(*_management, _eventHub);
	if(dialog.run() == Gtk::RESPONSE_OK)
	{
		Assign(dialog.SelectedInputPreset(), true);
	}
	return true;
}

void FaderWidget::Assign(PresetValue* item, bool moveFader)
{
	if(item != _preset)
	{
		_preset = item;
		if(_preset != nullptr)
		{
			_nameLabel.set_text(_preset->Title());
			if(moveFader)
			{
				immediateAssign(_preset->Value().UInt());
				_scale.set_value(_preset->Value().UInt());
			}
			else
				writeValue(_scale.get_value());
		}
		else {
			_nameLabel.set_text("<..>");
			if(moveFader)
			{
				immediateAssign(0);
				_scale.set_value(0);
			}
			else
				writeValue(_scale.get_value());
		}
		SignalAssigned().emit();
		if(moveFader)
			SignalValueChange().emit(_scale.get_value());
	}
}

void FaderWidget::MoveSlider()
{
	if(_preset != nullptr)
	{
		immediateAssign(_preset->Value().UInt());
		_scale.set_value(_preset->Value().UInt());
		SignalValueChange().emit(_scale.get_value());
	}
}

void FaderWidget::onUpdate()
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

void FaderWidget::Toggle()
{
	_onCheckButton.set_active(!_onCheckButton.get_active());
}

void FaderWidget::FullOn()
{
	_scale.set_value(ControlValue::MaxUInt());
}

void FaderWidget::FullOff()
{
	_scale.set_value(0);
}

void FaderWidget::ChangeManagement(class Management& management, bool moveSliders)
{
	if(_preset == nullptr)
	{
		_management = &management;
	}
	else {
		std::string controllablePath = _preset->Controllable().FullPath();
		size_t input = _preset->InputIndex();
		_management = &management;
		Controllable* controllable =
			dynamic_cast<Controllable*>(_management->GetObjectFromPathIfExists(controllablePath));
		PresetValue* pv;
		if(controllable)
			pv = _management->GetPresetValue(*controllable, input);
		else
			pv = nullptr;
		if(pv == nullptr)
			Unassign();
		else {
			Assign(pv, moveSliders);
		}
	}
}
