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

#include "avoidrecursion.h"
#include "nameframe.h"

#include "components/objecttree.h"

/**
	@author Andre Offringa
*/
class SequenceFrame : public Gtk::VPaned {
	public:
		SequenceFrame(class Management& management, class ShowWindow& parentWindow);
		~SequenceFrame();

		void ChangeManagement(class Management& management)
		{
			_nameFrame.ChangeManagement(management);
			_management = &management;
		}
		
		void Select(const class Sequence& sequence);
	private:
		void onCreateChaseButtonClicked();
		void onSelectedSequenceChanged();

		Gtk::Frame _sequenceFrame;
		ObjectTree _sequenceList;
		Gtk::HBox _sequenceInnerBox;
		Gtk::VBox _sequenceOuterBox;
		Gtk::ScrolledWindow _sequenceScrolledWindow;
		Gtk::VButtonBox _sequenceButtonBox;
		Gtk::Button _createChaseButton;

		AvoidRecursion _delayUpdates;
		NameFrame _nameFrame;

		class Management* _management;
		class ShowWindow& _parentWindow;
};

#endif
