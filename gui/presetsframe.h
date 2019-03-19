#ifndef PRESETSFRAME_H
#define PRESETSFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>

#include "avoidrecursion.h"
#include "nameframe.h"

#include "components/objecttree.h"

/**
	@author Andre Offringa
*/
class PresetsFrame : public Gtk::VPaned
{
public:
	PresetsFrame(class Management& management, class ShowWindow& parentWindow);

	void ChangeManagement(class Management &management)
	{
		_nameFrame.ChangeManagement(management);
		_management = &management;
	}
	
private:
	void initNewSequencePart();
	void initPresetsPart();

	void onNewPresetButtonClicked();
	void onNewFolderButtonClicked();
	void onCreateChaseButtonClicked();
	void onDeletePresetButtonClicked();
	void onAddPresetToSequenceButtonClicked();
	void onClearSequenceButtonClicked();
	void onCreateSequenceButtonClicked();
	void onSelectedPresetChanged();
	
	Gtk::Frame _presetsFrame;
	ObjectTree _presetsList;
	
	Gtk::TreeView _newSequenceListView;
	Glib::RefPtr<Gtk::ListStore> _newSequenceListModel;
	struct NewSequenceListColumns : public Gtk::TreeModelColumnRecord
	{
		NewSequenceListColumns()
			{ add(_title); add(_preset); }
	
		Gtk::TreeModelColumn<Glib::ustring> _title;
		Gtk::TreeModelColumn<class PresetCollection *> _preset;
	} _newSequenceListColumns;

	Gtk::VBox _presetsVBox;
	Gtk::HBox _presetsHBox, _newSequenceBox;

	Gtk::Frame _newSequenceFrame;

	Gtk::ScrolledWindow _newSequenceScrolledWindow;

	Gtk::VButtonBox _presetsButtonBox, _newSequenceButtonBox;
	Gtk::Button _newPresetButton, _newFolderButton, _createChaseButton, _deletePresetButton;
	Gtk::Button _addPresetToSequenceButton, _clearSequenceButton, _createSequenceButton;

	Management* _management;
	class ShowWindow& _parentWindow;
	NameFrame _nameFrame;
	AvoidRecursion _delayUpdates;
};

#endif
