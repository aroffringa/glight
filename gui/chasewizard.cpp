#include "chasewizard.h"
#include "showwindow.h"

#include "../libtheatre/fixture.h"
#include "../libtheatre/management.h"
#include "../libtheatre/theatre.h"

#include <memory>

ChaseWizard::ChaseWizard(ShowWindow* showWindow) :
	_management(&showWindow->GetManagement()),
	_selectLabel("Select fixtures:"),
	_nextButton("Next")
{
	showWindow->SignalChangeManagement().connect(sigc::mem_fun(*this, &ChaseWizard::onManagementChange));
	showWindow->SignalUpdateControllables().connect(sigc::mem_fun(*this, &ChaseWizard::fillFixturesList));
	
	_vBox.pack_start(_selectLabel);
	
	_fixturesListModel = Gtk::ListStore::create(_fixturesListColumns);
	_fixturesListView.set_model(_fixturesListModel);
	_fixturesListView.append_column("Fixture", _fixturesListColumns._title);
	_fixturesListView.append_column("Type", _fixturesListColumns._type);
	_fixturesListView.set_rubber_banding(true);
	_fixturesListView.get_selection()->set_mode(Gtk::SelectionMode::SELECTION_MULTIPLE);
	fillFixturesList();
	_fixturesScrolledWindow.add(_fixturesListView);
	_fixturesScrolledWindow.set_size_request(300, 400);
	_vBox.pack_start(_fixturesScrolledWindow);
	
	_buttonBox.pack_start(_nextButton);
	_vBox.pack_end(_buttonBox);
	
	add(_vBox);
	show_all_children();
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
