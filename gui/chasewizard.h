#ifndef CHASE_WIZARD_H
#define CHASE_WIZARD_H

#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/drawingarea.h>
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
	void onNextClicked();
	void onColorClicked();
	bool onColorAreaDraw(const Cairo::RefPtr<Cairo::Context>& cr);
	void initPage2();
	void initPage3();
	void makeChase();
	
	class ShowWindow* _showWindow;
	class Management* _management;
	
	Gtk::VBox _mainBox;
	Gtk::VBox _vBoxPage2, _vBoxPage3;
	Gtk::Label _selectLabel;
	Gtk::TreeView _fixturesListView;
	std::vector<class Fixture*> _selectedFixtures;
	
	Gtk::HBox _colorHBox;
	Gtk::Label _colorLabel;
	Gtk::DrawingArea _colorArea;
	Gtk::Button _colorButton;
	double _colorR, _colorG, _colorB;
	
	Gtk::ButtonBox _buttonBox;
	Gtk::Button _nextButton;
	int _currentPage;
	
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
