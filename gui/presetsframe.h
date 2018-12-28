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
#include <gtkmm/treeview.h>

#include "nameframe.h"

/**
	@author Andre Offringa
*/
class PresetsFrame : public Gtk::VPaned
{
	public:
		PresetsFrame(class Management &management, class ProgramWindow &parentWindow);
		~PresetsFrame();

		void Update() { FillPresetsList(); }
		void UpdateAfterPresetRemoval() { FillPresetsList(); }	
private:
		void FillPresetsList();

		void initNewSequencePart();
		void initPresetsPart();

		void onNewPresetButtonClicked();
		void onDeletePresetButtonClicked();
		void onAddPresetToSequenceButtonClicked();
		void onClearSequenceButtonClicked();
		void onCreateSequenceButtonClicked();
		void onSelectedPresetChanged();
		void onNameChange() { FillPresetsList(); }

		Gtk::TreeView _presetListView;
		Glib::RefPtr<Gtk::ListStore> _presetListModel;
		struct PresetListColumns : public Gtk::TreeModelColumnRecord
		{
			PresetListColumns()
				{ add(_title); add(_preset); }
		
			Gtk::TreeModelColumn<Glib::ustring> _title;
			Gtk::TreeModelColumn<class PresetCollection *> _preset;
		} _presetListColumns;

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
		Gtk::Frame _presetsFrame;

		Gtk::Frame _newSequenceFrame;

		Gtk::ScrolledWindow _presetsScrolledWindow, _newSequenceScrolledWindow;

		Gtk::VButtonBox _presetsButtonBox, _newSequenceButtonBox;
		Gtk::Button _newPresetButton, _deletePresetButton;
		Gtk::Button _addPresetToSequenceButton, _clearSequenceButton, _createSequenceButton;

		Management &_management;
		class ProgramWindow &_parentWindow;
		NameFrame _nameFrame;
};

#endif
