#include "chasewizard.h"
#include "showwindow.h"

#include "components/colorselectwidget.h"

#include "../theatre/defaultchase.h"
#include "../theatre/fixture.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"

#include <memory>

ChaseWizard::ChaseWizard(ShowWindow* showWindow) :
	_showWindow(showWindow),
	_management(&showWindow->GetManagement()),
	_selectLabel("Select fixtures:"),
	_runningLightBtn("Running light"),
	_singleColourBtn("Random around single colour"),
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
	_vuIncreasingRB("Increasing direction"),
	_vuDecreasingRB("Decreasing direction"),
	_vuInwardRunRB("Inward direction"),
	_vuOutwardRunRB("Outward direcion"),
	_nextButton("Next"),
	_currentPage(Page1_SelFixtures)
{
	showWindow->SignalChangeManagement().connect(sigc::mem_fun(*this, &ChaseWizard::onManagementChange));
	showWindow->SignalUpdateControllables().connect(sigc::mem_fun(*this, &ChaseWizard::fillFixturesList));
	
	initPage1();
	initPage2();
	initPage3_1RunningLight();
	initPage3_2SingleColour();
	initPage3_3VUMeter();
	
	_mainBox.pack_start(_vBoxPage1, true, true);
	_vBoxPage1.show_all();
	
	_nextButton.signal_clicked().connect(sigc::mem_fun(*this, &ChaseWizard::onNextClicked));
	_buttonBox.pack_start(_nextButton);
	_buttonBox.show_all();
	_mainBox.pack_end(_buttonBox);
	
	add(_mainBox);
	_mainBox.show();
}

ChaseWizard::~ChaseWizard()
{ }

void ChaseWizard::initPage1()
{
	_vBoxPage1.pack_start(_selectLabel);
	
	_fixturesListModel = Gtk::ListStore::create(_fixturesListColumns);
	_fixturesListView.set_model(_fixturesListModel);
	_fixturesListView.append_column("Fixture", _fixturesListColumns._title);
	_fixturesListView.append_column("Type", _fixturesListColumns._type);
	_fixturesListView.set_rubber_banding(true);
	_fixturesListView.get_selection()->set_mode(Gtk::SelectionMode::SELECTION_MULTIPLE);
	fillFixturesList();
	_fixturesScrolledWindow.add(_fixturesListView);
	_fixturesScrolledWindow.set_size_request(300, 400);
	_vBoxPage1.pack_start(_fixturesScrolledWindow);
}

void ChaseWizard::initPage2()
{
	Gtk::RadioButtonGroup group;
	_runningLightBtn.set_group(group);
	_vBoxPage2.pack_start(_runningLightBtn);
	_singleColourBtn.set_group(group);
	_vBoxPage2.pack_start(_singleColourBtn);
	_vuMeterBtn.set_group(group);
	_vBoxPage2.pack_start(_vuMeterBtn);
}

void ChaseWizard::initPage3_1RunningLight()
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

void ChaseWizard::initPage3_2SingleColour()
{
	_vBoxPage3_2.pack_start(_colorsWidgetP3_2, true, false);
	_vBoxPage3_2.pack_start(_variationLabel, true, false);
	_variation.set_range(0, 100);
	_variation.set_increments(1.0, 10.0);
	_vBoxPage3_2.pack_start(_variation, true, false);
}

void ChaseWizard::initPage3_3VUMeter()
{
	_vBoxPage3_3.pack_start(_colorsWidgetP3_3, true, false);
	Gtk::RadioButtonGroup group;
	_vuIncreasingRB.set_group(group);
	_vBoxPage3_3.pack_start(_vuIncreasingRB, true, false);
	_vuDecreasingRB.set_group(group);
	_vBoxPage3_3.pack_start(_vuDecreasingRB, true, false);
	_vuInwardRunRB.set_group(group);
	_vBoxPage3_3.pack_start(_vuInwardRunRB, true, false);
	_vuOutwardRunRB.set_group(group);
	_vBoxPage3_3.pack_start(_vuOutwardRunRB, true, false);
}

void ChaseWizard::fillFixturesList()
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

void ChaseWizard::onNextClicked()
{
	switch(_currentPage)
	{
		case Page1_SelFixtures: {
		_selectedFixtures.clear();
		Glib::RefPtr<Gtk::TreeSelection> selection =
			_fixturesListView.get_selection();
		auto selected = selection->get_selected_rows();
		for(auto& row : selected)
		{
			_selectedFixtures.emplace_back((*_fixturesListModel->get_iter(row))[_fixturesListColumns._fixture]);
		}
		_mainBox.remove(_vBoxPage1);
		_mainBox.pack_start(_vBoxPage2, true, true);
		_vBoxPage2.show_all();
		_currentPage = Page2_SelType;
		} break;
		
		case Page2_SelType:
		_mainBox.remove(_vBoxPage2);
		if(_runningLightBtn.get_active())
		{
			_colorsWidgetP3_1.SetMaxCount(_selectedFixtures.size());
			_mainBox.pack_start(_vBoxPage3_1, true, true);
			_vBoxPage3_1.show_all();
			_currentPage = Page3_1_RunningLight;
		}
		else if(_singleColourBtn.get_active()) {
			_mainBox.pack_start(_vBoxPage3_2, true, true);
			_vBoxPage3_2.show_all();
			_currentPage = Page3_2_SingleColour;
		}
		else {
			_colorsWidgetP3_3.SetMinCount(_selectedFixtures.size());
			_colorsWidgetP3_3.SetMaxCount(_selectedFixtures.size());
			_mainBox.pack_start(_vBoxPage3_3, true, true);
			_vBoxPage3_3.show_all();
			_currentPage = Page3_3_VUMeter;
		}
		break;
		
		case Page3_1_RunningLight: {
			_mainBox.remove(_vBoxPage3_1);
			_mainBox.pack_start(_vBoxPage1, true, true);
			_vBoxPage1.show_all();
			_currentPage = Page1_SelFixtures;
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
			DefaultChase::MakeRunningLight(*_management, _selectedFixtures, _colorsWidgetP3_1.GetColors(), runType);
			_showWindow->EmitUpdate();
			hide();
		} break;
		
		case Page3_2_SingleColour:
		_mainBox.remove(_vBoxPage3_2);
		_mainBox.pack_start(_vBoxPage1, true, true);
		_vBoxPage1.show_all();
		_currentPage = Page1_SelFixtures;
		DefaultChase::MakeColorVariation(*_management, _selectedFixtures, _colorsWidgetP3_2.GetColors(), _variation.get_value());
		_showWindow->EmitUpdate();
		hide();
		break;
		
		case Page3_3_VUMeter: {
			_mainBox.remove(_vBoxPage3_3);
			_mainBox.pack_start(_vBoxPage1, true, true);
			_vBoxPage1.show_all();
			_currentPage = Page1_SelFixtures;
			DefaultChase::VUMeterDirection direction;
			if(_vuIncreasingRB.get_active())
				direction = DefaultChase::VUIncreasing;
			else if(_vuDecreasingRB.get_active())
				direction = DefaultChase::VUDecreasing;
			else if(_vuInwardRunRB.get_active())
				direction = DefaultChase::VUInward;
			else //if(_vuOutwardRunRB.get_active())
				direction = DefaultChase::VUOutward;
			DefaultChase::MakeVUMeter(*_management, _selectedFixtures, _colorsWidgetP3_3.GetColors(), direction);
			_showWindow->EmitUpdate();
			hide();
		} break;
	}
}
