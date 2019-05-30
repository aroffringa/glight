#ifndef PRESETSFRAME_H
#define PRESETSFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

#include "avoidrecursion.h"
#include "nameframe.h"
#include "propertieswindow.h"
#include "windowlist.h"

#include "components/objectbrowser.h"

#include "../libtheatre/effect.h"

/**
	@author Andre Offringa
*/
class PresetsFrame : public Gtk::VPaned
{
public:
	PresetsFrame(class Management& management, class ShowWindow& parentWindow);

private:
	void initPresetsPart();

	void onNewPresetButtonClicked();
	void onNewChaseButtonClicked();
	bool onNewEffectButtonClicked(GdkEventButton* event);
	void onNewFolderButtonClicked();
	void onDeletePresetButtonClicked();
	void onSelectedPresetChanged();
	void onObjectActivated(class FolderObject& object);
	void onNewEffectMenuClicked(enum Effect::Type effectType);
	
	void changeManagement(class Management &management)
	{
		_nameFrame.ChangeManagement(management);
		_management = &management;
	}
	
	Gtk::Frame _presetsFrame;
	ObjectBrowser _list;
	
	Gtk::VBox _presetsVBox;
	Gtk::HBox _presetsHBox;

	Gtk::VButtonBox _presetsButtonBox;
	Gtk::Button _newPresetButton, _newChaseButton, _newEffectButton, _newFolderButton, _deletePresetButton;

	std::unique_ptr<Gtk::Menu> _popupEffectMenu;
	std::vector<std::unique_ptr<Gtk::MenuItem>> _popupEffectMenuItems;
		
	WindowList<PropertiesWindow> _windowList;
	
	Management* _management;
	class ShowWindow& _parentWindow;
	NameFrame _nameFrame;
	AvoidRecursion _delayUpdates;
};

#endif
