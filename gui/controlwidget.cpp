#include "controlwidget.h"

#include "../libtheatre/chase.h"
#include "../libtheatre/presetcollection.h"
#include "../libtheatre/presetvalue.h"
#include "../libtheatre/management.h"
#include "../libtheatre/controllable.h"

#define MAX_SCALE_VALUE_DEF (1<<24)

ControlWidget::ControlWidget(class Management &management, char key)
 :
  _scale(0, MAX_SCALE_VALUE_DEF, MAX_SCALE_VALUE_DEF/100),
	_flashButton(std::string(1, key)), _nameLabel("<..>"),
	_management(management), _preset(nullptr),
	_holdUpdates(false)
{
	_scale.set_inverted(true);
	_scale.set_draw_value(false);
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

	_onCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ControlWidget::onOnButtonClicked));
	pack_start(_onCheckButton, false, false, 0);
	_onCheckButton.show();

	pack_start(_eventBox, false, false, 0);
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
		_signalChange.emit(_scale.get_value());
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
		_signalChange.emit(_scale.get_value());
	}
}

bool ControlWidget::onNameLabelClicked(GdkEventButton* event)
{
	_popupMenu.reset(new Gtk::Menu());
	_popupChaseMenu.reset(new Gtk::Menu());
	_popupPresetMenu.reset(new Gtk::Menu());
	_popupFunctionMenu.reset(new Gtk::Menu());
	_popupMenuItems.clear();
	
	std::unique_ptr<Gtk::MenuItem> submi;
	
	submi.reset(new Gtk::MenuItem("Functions"));
	submi->set_submenu(*_popupFunctionMenu);
	_popupMenu->append(*submi);
	_popupMenuItems.emplace_back(std::move(submi));

	submi.reset(new Gtk::MenuItem("Presets"));
	submi->set_submenu(*_popupPresetMenu);
	_popupMenu->append(*submi);
	_popupMenuItems.emplace_back(std::move(submi));

	submi.reset(new Gtk::MenuItem("Chases"));
	submi->set_submenu(*_popupChaseMenu);
	_popupMenu->append(*submi);
	_popupMenuItems.emplace_back(std::move(submi));

	const std::vector<std::unique_ptr<PresetValue>>&
		presets = _management.PresetValues();
	for(const std::unique_ptr<PresetValue>& pv : presets)
	{
		Gtk::Menu* subMenu;
		Controllable& c = pv->Controllable();
		if(dynamic_cast<Chase*>(&c) != nullptr)
			subMenu = _popupChaseMenu.get();
		else if(dynamic_cast<PresetCollection*>(&c))
			subMenu = _popupPresetMenu.get();
		else
			subMenu = _popupFunctionMenu.get();
		
		std::unique_ptr<Gtk::MenuItem> mi(new Gtk::MenuItem(c.Name()));
		mi->signal_activate().connect(sigc::bind<PresetValue*>( 
    sigc::mem_fun(*this, &ControlWidget::onMenuItemClicked), pv.get()));
		subMenu->append(*mi);
		_popupMenuItems.emplace_back(std::move(mi));
	}
	_popupMenu->show_all_children();
	_popupMenu->popup(event->button, event->time);

	return true;
}

void ControlWidget::writeValue()
{
	if(_preset != nullptr)
	{
		_preset->Value().Set((unsigned) _scale.get_value());
	}
}

void ControlWidget::Assign(PresetValue* item)
{
	if(item != _preset)
	{
		if(item != nullptr)
			_nameLabel.set_text(item->Controllable().Name());
		else
			_nameLabel.set_text("<..>");
		if(_preset != 0)
			_preset->Value().Set(0);
		_preset = item;
		_scale.set_value(0.0);
	}
}

void ControlWidget::onMenuItemClicked(PresetValue *item)
{
	Assign(item);
}

void ControlWidget::UpdateAfterPresetRemoval()
{
	if(_preset != nullptr)
	{
		if(!_management.Contains(*_preset))
		{
			_nameLabel.set_text("<..>");
			_preset = nullptr;
			_scale.set_value(0.0);
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
