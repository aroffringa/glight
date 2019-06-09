#ifndef CONTROLLABLE_SELECT_MENU_H

#include <memory>
#include <vector>

#include <gtkmm/menu.h>

#include <sigc++/signal.h>

class InputSelectMenu
{
public:
	void Popup(class Management& management, GdkEventButton* event);
	
	sigc::signal<void(class PresetValue*)>& SignalInputSelected()
	{
		return _signalInputSelected;
	}
	
private:
	void onMenuItemClicked(class PresetValue *item);

	std::vector<std::unique_ptr<Gtk::MenuItem>> _popupMenuItems;
	std::unique_ptr<Gtk::Menu>
		_popupMenu,
		_popupChaseMenu,
		_popupSequenceMenu,
		_popupPresetMenu,
		_popupFunctionMenu,
		_popupEffectsMenu;
	sigc::signal<void(class PresetValue*)> _signalInputSelected;
};

#endif
