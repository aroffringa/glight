#ifndef CHASE_WIZARD_H
#define CHASE_WIZARD_H

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

class ChaseWizard : public Gtk::Window
{
public:
	ChaseWizard(class ShowWindow*);
	
private:
	void fillFixturesList();
	void onManagementChange(class Management& newManagement) { _management = &newManagement; fillFixturesList(); }
	
	class Management* _management;
	
	Gtk::VBox _vBox;
	Gtk::Label _selectLabel;
	Gtk::TreeView _fixturesListView;
	Gtk::ButtonBox _buttonBox;
	Gtk::Button _nextButton;
	
	Glib::RefPtr<Gtk::ListStore> _fixturesListModel;
	struct FixturesListColumns : public Gtk::TreeModelColumnRecord
	{
		FixturesListColumns()
			{ add(_title); add(_type); add(_fixture); }
	
		Gtk::TreeModelColumn<Glib::ustring> _title, _type;
		Gtk::TreeModelColumn<class Fixture*> _fixture;
	} _fixturesListColumns;
	Gtk::ScrolledWindow _fixturesScrolledWindow;
};

#endif // CHASE_WIZARD_H
