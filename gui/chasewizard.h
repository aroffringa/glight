#ifndef CHASE_WIZARD_H
#define CHASE_WIZARD_H

#include "components/colorsequencewidget.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include <vector>

class ChaseWizard : public Gtk::Window
{
public:
	ChaseWizard(class ShowWindow*);
	~ChaseWizard();
	
private:
	void fillFixturesList();
	void onManagementChange(class Management& newManagement)
	{
		_management = &newManagement;
		fillFixturesList();
	}
	void onNextClicked();
	void initPage1();
	void initPage2();
	void initPage3();
	void initPage4();
	
	class ShowWindow* _showWindow;
	class Management* _management;
	
	Gtk::VBox _mainBox;
	Gtk::VBox _vBoxPage1, _vBoxPage2, _vBoxPage3, _vBoxPage4;
	Gtk::Label _selectLabel;
	Gtk::TreeView _fixturesListView;
	std::vector<class Fixture*> _selectedFixtures;
	
	Gtk::RadioButton
		_runningLightBtn,
		_randomAroundSingleColourBtn;
	
	ColorSequenceWidget _colorsWidgetP3;
	Gtk::RadioButton
		_increasingRunRB,
		_decreasingRunRB,
		_backAndForthRunRB,
		_inwardRunRB,
		_outwardRunRB,
		_randomRunRB;
	
	ColorSequenceWidget _colorsWidgetP4;
	Gtk::Label _variationLabel;
	Gtk::Scale _variation;
	
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
