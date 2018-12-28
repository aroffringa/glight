#ifndef SEQUENCEFRAME_H
#define SEQUENCEFRAME_H

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
class SequenceFrame : public Gtk::VPaned {
	public:
		SequenceFrame(class Management &management, class ProgramWindow &parentWindow);
		~SequenceFrame();

		void Update() { fillSequenceList(); }
		void UpdateAfterPresetRemoval() { fillSequenceList(); }
	private:
		void fillSequenceList();
		void onCreateChaseButtonClicked();
		void onSelectedSequenceChanged();
		void onNameChange() { fillSequenceList(); }

		struct SequenceListColumns : public Gtk::TreeModelColumnRecord
		{
			SequenceListColumns()
				{ add(_title); add(_sequence); }
		
			Gtk::TreeModelColumn<Glib::ustring> _title;
			Gtk::TreeModelColumn<class Sequence *> _sequence;
		} _sequenceListColumns;
		Gtk::TreeView _sequenceListView;
		Glib::RefPtr<Gtk::ListStore> _sequenceListModel;
		Gtk::HBox _sequenceInnerBox;
		Gtk::VBox _sequenceOuterBox;
		Gtk::ScrolledWindow _sequenceScrolledWindow;
		Gtk::VButtonBox _sequenceButtonBox;
		Gtk::Frame _sequenceFrame;
		Gtk::Button _createChaseButton;

		NameFrame _nameFrame;

		class Management &_management;
		class ProgramWindow &_parentWindow;
};

#endif
