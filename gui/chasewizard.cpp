#include "chasewizard.h"
#include "showwindow.h"

#include "components/colorselectwidget.h"

#include "../libtheatre/defaultchase.h"
#include "../libtheatre/fixture.h"
#include "../libtheatre/management.h"
#include "../libtheatre/theatre.h"

#include <memory>

ChaseWizard::ChaseWizard(ShowWindow* showWindow) :
	_showWindow(showWindow),
	_management(&showWindow->GetManagement()),
	_selectLabel("Select fixtures:"),
	_runningLightBtn("Running light"),
	_randomAroundSingleColourBtn("Random around single colour"),
	_colorsWidgetP3(this),
	_colorsWidgetP4(this),
	_variationLabel("Variation:"),
	_nextButton("Next"),
	_currentPage(1)
{
	showWindow->SignalChangeManagement().connect(sigc::mem_fun(*this, &ChaseWizard::onManagementChange));
	showWindow->SignalUpdateControllables().connect(sigc::mem_fun(*this, &ChaseWizard::fillFixturesList));
	
	initPage1();
	initPage2();
	initPage3();
	initPage4();
	
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
	_randomAroundSingleColourBtn.set_group(group);
	_vBoxPage2.pack_start(_randomAroundSingleColourBtn);
}

void ChaseWizard::initPage3()
{
	_vBoxPage3.pack_start(_colorsWidgetP3, true, false);
}

void ChaseWizard::initPage4()
{
	_vBoxPage4.pack_start(_colorsWidgetP4, true, false);
	_vBoxPage4.pack_start(_variationLabel, true, false);
	_variation.set_range(0, 100);
	_variation.set_increments(1.0, 10.0);
	_vBoxPage4.pack_start(_variation, true, false);
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
		case 1: {
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
		_currentPage = 2;
		} break;
		case 2:
		_mainBox.remove(_vBoxPage2);
		if(_runningLightBtn.get_active())
		{
			_mainBox.pack_start(_vBoxPage3, true, true);
			_vBoxPage3.show_all();
			_currentPage = 3;
		}
		else {
			_mainBox.pack_start(_vBoxPage4, true, true);
			_vBoxPage4.show_all();
			_currentPage = 4;
		}
		break;
		case 3:
		_mainBox.remove(_vBoxPage3);
		_mainBox.pack_start(_vBoxPage1, true, true);
		_vBoxPage1.show_all();
		_currentPage = 1;
		DefaultChase::MakeRunningLight(*_management, _selectedFixtures, _colorsWidgetP3.GetColors());
		_showWindow->EmitUpdate();
		hide();
		break;
		case 4:
		_mainBox.remove(_vBoxPage4);
		_mainBox.pack_start(_vBoxPage1, true, true);
		_vBoxPage1.show_all();
		_currentPage = 1;
		DefaultChase::MakeColorVariation(*_management, _selectedFixtures, _colorsWidgetP4.GetColors(), _variation.get_value());
		_showWindow->EmitUpdate();
		hide();
		break;
	}
}
