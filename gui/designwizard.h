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

class DesignWizard : public Gtk::Window
{
public:
	DesignWizard(class ShowWindow*, const std::string& destinationPath);
	~DesignWizard();
	
	void SetDestinationPath(const std::string& destinationPath)
	{
		_destinationPath = destinationPath;
	}
	
private:
	enum Page {
		Page1_SelFixtures,
		Page2_SelType,
		Page3_1_RunningLight,
		Page3_2_SingleColor,
		Page3_3_ShiftingColors,
		Page3_4_VUMeter,
		Page3_5_ColorPreset
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
	void initPage3_2SingleColor();
	void initPage3_3ShiftColors();
	void initPage3_4VUMeter();
	void initPage3_5ColorPreset();
	class Folder& getFolder() const;
	
	class ShowWindow* _showWindow;
	class Management* _management;
	std::string _destinationPath;
	
	Gtk::VBox _mainBox;
	Gtk::VBox _vBoxPage1, _vBoxPage2, _vBoxPage3_1, _vBoxPage3_2, _vBoxPage3_3, _vBoxPage3_4, _vBoxPage3_5;
	Gtk::Label _selectLabel;
	Gtk::TreeView _fixturesListView;
	std::vector<class Fixture*> _selectedFixtures;
	
	Gtk::RadioButton
		_colorPresetBtn,
		_runningLightBtn,
		_singleColorBtn,
		_shiftColorsBtn,
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
		
	ColorSequenceWidget _colorsWidgetP3_5;
	
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