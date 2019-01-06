#ifndef CONTROLLABLE_SELECT_MENU_H

#include <memory>
#include <vector>

#include <gtkmm/menu.h>

#include <sigc++/signal.h>

class ControllableSelectMenu
{
public:
	void Popup(class Management& management, GdkEventButton* event);
	
	sigc::signal<void(class PresetValue*)>& SignalControllableSelected()
	{
		return _signalControllableSelected;
	}
	
private:
	void onMenuItemClicked(class PresetValue *item);

	std::vector<std::unique_ptr<Gtk::MenuItem>> _popupMenuItems;
	std::unique_ptr<Gtk::Menu> _popupMenu, _popupChaseMenu, _popupPresetMenu, _popupFunctionMenu;
	sigc::signal<void(class PresetValue*)> _signalControllableSelected;
};

#endif
