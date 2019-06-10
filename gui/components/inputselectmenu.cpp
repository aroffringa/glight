#include "inputselectmenu.h"

#include "../../theatre/chase.h"
#include "../../theatre/controllable.h"
#include "../../theatre/effect.h"
#include "../../theatre/management.h"
#include "../../theatre/presetcollection.h"
#include "../../theatre/presetvalue.h"
#include "../../theatre/timesequence.h"

void InputSelectMenu::Popup(Management& management, GdkEventButton* event)
{
	_popupMenu.reset(new Gtk::Menu());
	_popupChaseMenu.reset(new Gtk::Menu());
	_popupSequenceMenu.reset(new Gtk::Menu());
	_popupPresetMenu.reset(new Gtk::Menu());
	_popupFunctionMenu.reset(new Gtk::Menu());
	_popupEffectsMenu.reset(new Gtk::Menu());
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
	
	submi.reset(new Gtk::MenuItem("Sequences"));
	submi->set_submenu(*_popupSequenceMenu);
	_popupMenu->append(*submi);
	_popupMenuItems.emplace_back(std::move(submi));
	
	submi.reset(new Gtk::MenuItem("Effects"));
	submi->set_submenu(*_popupEffectsMenu);
	_popupMenu->append(*submi);
	_popupMenuItems.emplace_back(std::move(submi));

	const std::vector<std::unique_ptr<PresetValue>>&
		presets = management.PresetValues();
	for(const std::unique_ptr<PresetValue>& pv : presets)
	{
		Gtk::Menu* subMenu;
		Controllable& c = pv->Controllable();
		if(dynamic_cast<Chase*>(&c) != nullptr)
			subMenu = _popupChaseMenu.get();
		else if(dynamic_cast<PresetCollection*>(&c))
			subMenu = _popupPresetMenu.get();
		else if(dynamic_cast<TimeSequence*>(&c))
			subMenu = _popupSequenceMenu.get();
		else if(dynamic_cast<Effect*>(&c))
			subMenu = _popupEffectsMenu.get();
		else
			subMenu = _popupFunctionMenu.get();
		
		std::unique_ptr<Gtk::MenuItem> mi(new Gtk::MenuItem(pv->Title()));
		mi->signal_activate().connect(sigc::bind<PresetValue*>( 
    sigc::mem_fun(*this, &InputSelectMenu::onMenuItemClicked), pv.get()));
		subMenu->append(*mi);
		_popupMenuItems.emplace_back(std::move(mi));
	}
	_popupMenu->show_all_children();
	_popupMenu->popup(event->button, event->time);
}

void InputSelectMenu::onMenuItemClicked(PresetValue* item)
{
	_signalInputSelected(item);
}
