#include "designwizard.h"
#include "eventtransmitter.h"

#include "components/colorselectwidget.h"

#include "../theatre/defaultchase.h"
#include "../theatre/fixture.h"
#include "../theatre/fixturecontrol.h"
#include "../theatre/folder.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"

#include <memory>

DesignWizard::DesignWizard(Management& management, EventTransmitter& hub, const std::string& destinationPath) :
	_eventHub(hub),
	_management(&management),
	_destinationPath(destinationPath),
	_selectLabel("Select fixtures:"),
	_objectBrowser(management, hub),
	
	_colorPresetBtn("Colour preset"),
	_runningLightBtn("Running light"),
	_singleColorBtn("Random around single colour"),
	_shiftColorsBtn("Shifting colours"),
	_vuMeterBtn("VU meter"),
	
	_colorsWidgetP3_1(this),
	_increasingRunRB("Increasing order"),
	_decreasingRunRB("Decreasing order"),
	_backAndForthRunRB("Back and forth"),
	_inwardRunRB("Inward"),
	_outwardRunRB("Outward"),
	_randomRunRB("Randomized"),
	_colorsWidgetP3_2(this),
	_variationLabel("Variation:"),
	_colorsWidgetP3_3(this),
	_shiftIncreasingRB("Increasing shift"),
	_shiftDecreasingRB("Decreasing shift"),
	_shiftBackAndForthRB("Back and forth"),
	_shiftRandomRB("Move randomly"),
	_colorsWidgetP3_4(this),
	_vuIncreasingRB("Increasing direction"),
	_vuDecreasingRB("Decreasing direction"),
	_vuInwardRunRB("Inward direction"),
	_vuOutwardRunRB("Outward direcion"),
	_colorsWidgetP3_5(this),
	_nextButton("Next"),
	_currentPage(Page1_SelFixtures)
{
	_eventHub.SignalChangeManagement().connect(sigc::mem_fun(*this, &DesignWizard::onManagementChange));
	_eventHub.SignalUpdateControllables().connect(sigc::mem_fun(*this, &DesignWizard::fillFixturesList));
	
	initPage1();
	initPage2();
	initPage3_1RunningLight();
	initPage3_2SingleColor();
	initPage3_3ShiftColors();
	initPage3_4VUMeter();
	initPage3_5ColorPreset();
	
	_mainBox.pack_start(_vBoxPage1, true, true);
	_vBoxPage1.show_all();
	
	_nextButton.signal_clicked().connect(sigc::mem_fun(*this, &DesignWizard::onNextClicked));
	_buttonBox.pack_start(_nextButton);
	_buttonBox.show_all();
	_mainBox.pack_end(_buttonBox);
	
	add(_mainBox);
	_mainBox.show();
}

DesignWizard::~DesignWizard()
{ }

void DesignWizard::initPage1()
{
	_vBoxPage1.pack_start(_notebook);
	
	_notebook.append_page(_vBoxPage1a, "Fixtures");
	_vBoxPage1a.pack_start(_selectLabel);
	
	_fixturesListModel = Gtk::ListStore::create(_fixturesListColumns);
	_fixturesListView.set_model(_fixturesListModel);
	_fixturesListView.append_column("Fixture", _fixturesListColumns._title);
	_fixturesListView.append_column("Type", _fixturesListColumns._type);
	_fixturesListView.set_rubber_banding(true);
	_fixturesListView.get_selection()->set_mode(Gtk::SelectionMode::SELECTION_MULTIPLE);
	fillFixturesList();
	_fixturesScrolledWindow.add(_fixturesListView);
	_fixturesScrolledWindow.set_size_request(300, 400);
	_vBoxPage1a.pack_start(_fixturesScrolledWindow);
	
	_notebook.append_page(_vBoxPage1b, "Any controllables");
	
	_objectBrowser.SignalSelectionChange().connect([&](){ onControllableSelected(); });
	_objectBrowser.SignalObjectActivated().connect(
		[&](FolderObject& object){ addControllable(object); });
	_vBoxPage1b.pack_start(_objectBrowser);
	
	_addControllableButton.set_image_from_icon_name("go-down");
	_addControllableButton.set_sensitive(false);
	_addControllableButton.signal_clicked().connect(sigc::mem_fun(*this, &DesignWizard::onAddControllable));
	_controllableButtonBox.pack_start(_addControllableButton, false, false, 4);
	
	_removeControllableButton.set_image_from_icon_name("go-up");
	_removeControllableButton.signal_clicked().connect(sigc::mem_fun(*this, &DesignWizard::onRemoveControllable));
	_controllableButtonBox.pack_start(_removeControllableButton, false, false, 4);
	
	_controllableButtonBox.set_halign(Gtk::ALIGN_CENTER);
	_vBoxPage1b.pack_start(_controllableButtonBox, false, false);
	
	_controllablesListModel = Gtk::ListStore::create(_controllablesListColumns);
	_controllablesListView.set_model(_controllablesListModel);
	_controllablesListView.append_column("Controllable", _fixturesListColumns._title);
	_controllablesListView.append_column("Path", _fixturesListColumns._type);
	_controllablesListView.set_rubber_banding(true);
	_controllablesListView.get_selection()->set_mode(Gtk::SelectionMode::SELECTION_MULTIPLE);
	_controllablesScrolledWindow.add(_controllablesListView);
	_vBoxPage1b.pack_end(_controllablesScrolledWindow, true, true);
}

void DesignWizard::initPage2()
{
	Gtk::RadioButtonGroup group;
	_colorPresetBtn.set_group(group);
	_vBoxPage2.pack_start(_colorPresetBtn);
	_runningLightBtn.set_group(group);
	_vBoxPage2.pack_start(_runningLightBtn);
	_singleColorBtn.set_group(group);
	_vBoxPage2.pack_start(_singleColorBtn);
	_shiftColorsBtn.set_group(group);
	_vBoxPage2.pack_start(_shiftColorsBtn);
	_vuMeterBtn.set_group(group);
	_vBoxPage2.pack_start(_vuMeterBtn);
}

void DesignWizard::initPage3_1RunningLight()
{
	_vBoxPage3_1.pack_start(_colorsWidgetP3_1, true, false);
	Gtk::RadioButtonGroup group;
	_increasingRunRB.set_group(group);
	_vBoxPage3_1.pack_start(_increasingRunRB, true, false);
	_decreasingRunRB.set_group(group);
	_vBoxPage3_1.pack_start(_decreasingRunRB, true, false);
	_backAndForthRunRB.set_group(group);
	_vBoxPage3_1.pack_start(_backAndForthRunRB, true, false);
	_inwardRunRB.set_group(group);
	_vBoxPage3_1.pack_start(_inwardRunRB, true, false);
	_outwardRunRB.set_group(group);
	_vBoxPage3_1.pack_start(_outwardRunRB, true, false);
	_randomRunRB.set_group(group);
	_vBoxPage3_1.pack_start(_randomRunRB, true, false);
}

void DesignWizard::initPage3_2SingleColor()
{
	_vBoxPage3_2.pack_start(_colorsWidgetP3_2, true, false);
	_vBoxPage3_2.pack_start(_variationLabel, true, false);
	_variation.set_range(0, 100);
	_variation.set_increments(1.0, 10.0);
	_vBoxPage3_2.pack_start(_variation, true, false);
}

void DesignWizard::initPage3_3ShiftColors()
{
	_vBoxPage3_3.pack_start(_colorsWidgetP3_3, true, false);
	Gtk::RadioButtonGroup group;
	_shiftIncreasingRB.set_group(group);
	_vBoxPage3_3.pack_start(_shiftIncreasingRB, true, false);
	_shiftDecreasingRB.set_group(group);
	_vBoxPage3_3.pack_start(_shiftDecreasingRB, true, false);
	_shiftBackAndForthRB.set_group(group);
	_vBoxPage3_3.pack_start(_shiftBackAndForthRB, true, false);
	_shiftRandomRB.set_group(group);
	_vBoxPage3_3.pack_start(_shiftRandomRB, true, false);
}

void DesignWizard::initPage3_4VUMeter()
{
	_vBoxPage3_4.pack_start(_colorsWidgetP3_4, true, false);
	Gtk::RadioButtonGroup group;
	_vuIncreasingRB.set_group(group);
	_vBoxPage3_4.pack_start(_vuIncreasingRB, true, false);
	_vuDecreasingRB.set_group(group);
	_vBoxPage3_4.pack_start(_vuDecreasingRB, true, false);
	_vuInwardRunRB.set_group(group);
	_vBoxPage3_4.pack_start(_vuInwardRunRB, true, false);
	_vuOutwardRunRB.set_group(group);
	_vBoxPage3_4.pack_start(_vuOutwardRunRB, true, false);
}

void DesignWizard::initPage3_5ColorPreset()
{
	_vBoxPage3_5.pack_start(_colorsWidgetP3_5, true, false);
}

void DesignWizard::fillFixturesList()
{
	_fixturesListModel->clear();

	std::lock_guard<std::mutex> lock(_management->Mutex());
	const std::vector<std::unique_ptr<Fixture>>
		&fixtures = _management->Theatre().Fixtures();
	for(const std::unique_ptr<Fixture>& fixture : fixtures)
	{
		Gtk::TreeModel::iterator iter = _fixturesListModel->append();
		Gtk::TreeModel::Row row = *iter;
		row[_fixturesListColumns._title] = fixture->Name();
		row[_fixturesListColumns._type] = fixture->Type().Name();
		row[_fixturesListColumns._fixture] = fixture.get();
	}
}

Folder& DesignWizard::getFolder() const
{
	Folder* folder = dynamic_cast<Folder*>(_management->GetObjectFromPathIfExists(_destinationPath));
	if(folder)
		return *folder;
	else
		return _management->RootFolder();
}

void DesignWizard::onNextClicked()
{
	switch(_currentPage)
	{
		case Page1_SelFixtures: {
		_selectedControllables.clear();
		if(_notebook.get_current_page()==0)
		{
			Glib::RefPtr<Gtk::TreeSelection> selection =
				_fixturesListView.get_selection();
			auto selected = selection->get_selected_rows();
			for(auto& row : selected)
			{
				Fixture* fixture =
					(*_fixturesListModel->get_iter(row))[_fixturesListColumns._fixture];
				_selectedControllables.emplace_back(&_management->GetFixtureControl(*fixture));
			}
		}
		else {
			for(auto& iter : _controllablesListModel->children())
			{
				_selectedControllables.emplace_back((*iter)[_controllablesListColumns._controllable]);
			}
		}
		_mainBox.remove(_vBoxPage1);
		_mainBox.pack_start(_vBoxPage2, true, true);
		_vBoxPage2.show_all();
		_currentPage = Page2_SelType;
		} break;
		
		case Page2_SelType:
		_mainBox.remove(_vBoxPage2);
		if(_colorPresetBtn.get_active())
		{
			_colorsWidgetP3_5.SetColors(std::vector<Color>(_selectedControllables.size(), Color::White()));
			_colorsWidgetP3_5.SetMinCount(_selectedControllables.size());
			_colorsWidgetP3_5.SetMaxCount(_selectedControllables.size());
			_mainBox.pack_start(_vBoxPage3_5, true, true);
			_vBoxPage3_5.show_all();
			_currentPage = Page3_5_ColorPreset;
		}
		else if(_runningLightBtn.get_active())
		{
			_colorsWidgetP3_1.SetColors(std::vector<Color>(_selectedControllables.size(), Color::White()));
			_colorsWidgetP3_1.SetMaxCount(_selectedControllables.size());
			_mainBox.pack_start(_vBoxPage3_1, true, true);
			_vBoxPage3_1.show_all();
			_currentPage = Page3_1_RunningLight;
		}
		else if(_singleColorBtn.get_active()) {
			_mainBox.pack_start(_vBoxPage3_2, true, true);
			_vBoxPage3_2.show_all();
			_currentPage = Page3_2_SingleColor;
		}
		else if(_shiftColorsBtn.get_active()) {
			_colorsWidgetP3_3.SetMinCount(_selectedControllables.size());
			_colorsWidgetP3_3.SetMaxCount(_selectedControllables.size());
			_mainBox.pack_start(_vBoxPage3_3, true, true);
			_vBoxPage3_3.show_all();
			_currentPage = Page3_3_ShiftingColors;
		}
		else {
			_colorsWidgetP3_4.SetMinCount(_selectedControllables.size());
			_colorsWidgetP3_4.SetMaxCount(_selectedControllables.size());
			_mainBox.pack_start(_vBoxPage3_4, true, true);
			_vBoxPage3_4.show_all();
			_currentPage = Page3_4_VUMeter;
		}
		break;
		
		case Page3_1_RunningLight: {
			enum DefaultChase::RunType runType;
			if(_increasingRunRB.get_active())
				runType = DefaultChase::IncreasingRun;
			else if(_decreasingRunRB.get_active())
				runType = DefaultChase::DecreasingRun;
			else if(_backAndForthRunRB.get_active())
				runType = DefaultChase::BackAndForthRun;
			else if(_inwardRunRB.get_active())
				runType = DefaultChase::InwardRun;
			else if(_outwardRunRB.get_active())
				runType = DefaultChase::OutwardRun;
			else //if(_randomRunRB.get_active())
				runType = DefaultChase::RandomRun;
			DefaultChase::MakeRunningLight(*_management, getFolder(), _selectedControllables, _colorsWidgetP3_1.GetColors(), runType);
			_eventHub.EmitUpdate();
			hide();
		} break;
		
		case Page3_2_SingleColor:
		DefaultChase::MakeColorVariation(*_management, getFolder(), _selectedControllables, _colorsWidgetP3_2.GetColors(), _variation.get_value());
		_eventHub.EmitUpdate();
		hide();
		break;
		
		case Page3_3_ShiftingColors: {
			enum DefaultChase::ShiftType shiftType;
			if(_shiftIncreasingRB.get_active())
				shiftType = DefaultChase::IncreasingShift;
			else if(_shiftDecreasingRB.get_active())
				shiftType = DefaultChase::DecreasingShift;
			else if(_shiftBackAndForthRB.get_active())
				shiftType = DefaultChase::BackAndForthShift;
			else
				shiftType = DefaultChase::RandomShift;
			DefaultChase::MakeColorShift(*_management, getFolder(), _selectedControllables, _colorsWidgetP3_3.GetColors(), shiftType);
			_eventHub.EmitUpdate();
			hide();
		} break;
		
		case Page3_4_VUMeter: {
			DefaultChase::VUMeterDirection direction;
			if(_vuIncreasingRB.get_active())
				direction = DefaultChase::VUIncreasing;
			else if(_vuDecreasingRB.get_active())
				direction = DefaultChase::VUDecreasing;
			else if(_vuInwardRunRB.get_active())
				direction = DefaultChase::VUInward;
			else //if(_vuOutwardRunRB.get_active())
				direction = DefaultChase::VUOutward;
			DefaultChase::MakeVUMeter(*_management, getFolder(), _selectedControllables, _colorsWidgetP3_4.GetColors(), direction);
			_eventHub.EmitUpdate();
			hide();
		} break;
		
		case Page3_5_ColorPreset: {
			DefaultChase::MakeColorPreset(*_management, getFolder(), _selectedControllables, _colorsWidgetP3_5.GetColors());
			_eventHub.EmitUpdate();
			hide();
		} break;
	}
}

void DesignWizard::addControllable(FolderObject& object)
{
	Controllable* controllable =
		dynamic_cast<Controllable*>(&object);
	if(controllable)
	{
		Gtk::TreeModel::iterator iter = _controllablesListModel->append();
		Gtk::TreeModel::Row row = *iter;
		if(iter)
		{
			row[_controllablesListColumns._controllable] = controllable;
			row[_controllablesListColumns._title] = controllable->Name();
			row[_controllablesListColumns._path] = controllable->Parent().FullPath();
		}
	}
}

void DesignWizard::onAddControllable()
{
	FolderObject* object = _objectBrowser.SelectedObject();
	if(object)
		addControllable(*object);
}

void DesignWizard::onRemoveControllable()
{
	std::vector<Gtk::TreeModel::Path> iter =
		_controllablesListView.get_selection()->get_selected_rows();

	for(std::vector<Gtk::TreeModel::Path>::reverse_iterator elementIter = iter.rbegin() ;
			elementIter != iter.rend(); ++elementIter)
		_controllablesListModel->erase(_controllablesListModel->get_iter(*elementIter));
}

void DesignWizard::onControllableSelected()
{
	Controllable* object =
		dynamic_cast<Controllable*>(_objectBrowser.SelectedObject());
	_addControllableButton.set_sensitive(object != nullptr);
}
