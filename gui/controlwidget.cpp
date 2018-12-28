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
	_popupMenu(0),
	_popupChaseMenu(0),
	_popupPresetMenu(0),
	_popupFunctionMenu(0),
	_management(management), _preset(0),
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
{
	DestroyPopupMenu();
}

double ControlWidget::MAX_SCALE_VALUE()
{
	return MAX_SCALE_VALUE_DEF;
}

void ControlWidget::DestroyPopupMenu()
{
	delete _popupMenu; _popupMenu = 0;
	delete _popupChaseMenu; _popupChaseMenu = 0;
	delete _popupFunctionMenu; _popupFunctionMenu = 0;
	delete _popupPresetMenu; _popupPresetMenu = 0;
	for(std::vector<Gtk::MenuItem*>::const_iterator i=_popupMenuItems.begin();
		i != _popupMenuItems.end();++i)
		delete *i;
	_popupMenuItems.clear();
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
	DestroyPopupMenu();

	_popupMenu = new Gtk::Menu();
	_popupChaseMenu = new Gtk::Menu();
	_popupPresetMenu = new Gtk::Menu();
	_popupFunctionMenu = new Gtk::Menu();
	
	Gtk::MenuItem *submi;
	
	submi = new Gtk::MenuItem("Functions");
	submi->set_submenu(*_popupFunctionMenu);
	_popupMenu->append(*submi);
	_popupMenuItems.push_back(submi);

	submi = new Gtk::MenuItem("Presets");
	submi->set_submenu(*_popupPresetMenu);
	_popupMenu->append(*submi);
	_popupMenuItems.push_back(submi);

	submi = new Gtk::MenuItem("Chases");
	submi->set_submenu(*_popupChaseMenu);
	_popupMenu->append(*submi);
	_popupMenuItems.push_back(submi);

	const std::vector<std::unique_ptr<PresetValue>>&
		presets = _management.PresetValues();
	for(const std::unique_ptr<PresetValue>& pv : presets)
	{
		Gtk::Menu* subMenu;
		Controllable& c = pv->Controllable();
		if(dynamic_cast<Chase*>(&c) != 0)
			subMenu = _popupChaseMenu;
		else if(dynamic_cast<PresetCollection*>(&c))
			subMenu = _popupPresetMenu;
		else
			subMenu = _popupFunctionMenu;
		
		Gtk::MenuItem *mi = new Gtk::MenuItem(c.Name());
		mi->signal_activate().connect(sigc::bind<PresetValue*>( 
    sigc::mem_fun(*this, &ControlWidget::onMenuItemClicked), pv.get()));
		_popupMenuItems.push_back(mi);

		subMenu->append(*mi);
	}
	_popupMenu->show_all_children();
	_popupMenu->popup(event->button, event->time);

	return true;
}

void ControlWidget::writeValue()
{
	if(_preset != 0)
	{
		_preset->Value().Set((unsigned) _scale.get_value());
	}
}

void ControlWidget::Assign(PresetValue* item)
{
	if(item != _preset)
	{
		if(item != 0)
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
	if(_preset != 0)
	{
		if(!_management.Contains(*_preset))
		{
			_nameLabel.set_text("<..>");
			_preset = 0;
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
