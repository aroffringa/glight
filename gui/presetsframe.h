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
#include "windowlist.h"

#include "components/objectbrowser.h"

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
	void onNewFolderButtonClicked();
	void onDeletePresetButtonClicked();
	void onSelectedPresetChanged();
	void onObjectActivated(class FolderObject& object);
	
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
	Gtk::Button _newPresetButton, _newChaseButton, _newFolderButton, _deletePresetButton;

	WindowList _windowList;
	
	Management* _management;
	class ShowWindow& _parentWindow;
	NameFrame _nameFrame;
	AvoidRecursion _delayUpdates;
};

#endif
