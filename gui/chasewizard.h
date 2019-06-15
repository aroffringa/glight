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
	ChaseWizard(class ShowWindow*, const std::string& destinationPath);
	~ChaseWizard();
	
	void SetDestinationPath(const std::string& destinationPath)
	{
		_destinationPath = destinationPath;
	}
	
private:
	enum Page {
		Page1_SelFixtures,
		Page2_SelType,
		Page3_1_RunningLight,
		Page3_2_SingleColour,
		Page3_3_ShiftingColours,
		Page3_4_VUMeter
	};
	
	void fillFixturesList();
	void onManagementChange(class Management& newManagement)
	{
		_management = &newManagement;
		fillFixturesList();
	}
	void onNextClicked();
	void initPage1();
	void initPage2();
	void initPage3_1RunningLight();
	void initPage3_2SingleColour();
	void initPage3_3ShiftColours();
	void initPage3_4VUMeter();
	class Folder& getFolder() const;
	
	class ShowWindow* _showWindow;
	class Management* _management;
	std::string _destinationPath;
	
	Gtk::VBox _mainBox;
	Gtk::VBox _vBoxPage1, _vBoxPage2, _vBoxPage3_1, _vBoxPage3_2, _vBoxPage3_3, _vBoxPage3_4;
	Gtk::Label _selectLabel;
	Gtk::TreeView _fixturesListView;
	std::vector<class Fixture*> _selectedFixtures;
	
	Gtk::RadioButton
		_runningLightBtn,
		_singleColourBtn,
		_shiftColoursBtn,
		_vuMeterBtn;
	
	ColorSequenceWidget _colorsWidgetP3_1;
	Gtk::RadioButton
		_increasingRunRB,
		_decreasingRunRB,
		_backAndForthRunRB,
		_inwardRunRB,
		_outwardRunRB,
		_randomRunRB;
	
	ColorSequenceWidget _colorsWidgetP3_2;
	Gtk::Label _variationLabel;
	Gtk::Scale _variation;
	
	ColorSequenceWidget _colorsWidgetP3_3;
	Gtk::RadioButton
		_shiftIncreasingRB,
		_shiftDecreasingRB,
		_shiftBackAndForthRB,
		_shiftRandomRB;
	
	ColorSequenceWidget _colorsWidgetP3_4;
	Gtk::RadioButton
		_vuIncreasingRB,
		_vuDecreasingRB,
		_vuInwardRunRB,
		_vuOutwardRunRB;
		
	Gtk::ButtonBox _buttonBox;
	Gtk::Button _nextButton;
	Page _currentPage;
	
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
