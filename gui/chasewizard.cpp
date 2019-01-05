#include "chasewizard.h"
#include "showwindow.h"

#include "../libtheatre/fixture.h"
#include "../libtheatre/fixturefunctioncontrol.h"
#include "../libtheatre/management.h"
#include "../libtheatre/presetcollection.h"
#include "../libtheatre/theatre.h"
#include "../libtheatre/sequence.h"

#include <gtkmm/colorchooserdialog.h>

#include <memory>

ChaseWizard::ChaseWizard(ShowWindow* showWindow) :
	_showWindow(showWindow),
	_management(&showWindow->GetManagement()),
	_selectLabel("Select fixtures:"),
	_colorLabel("Color:"),
	_colorButton("Change..."),
	_colorR(1.0), _colorG(1.0), _colorB(1.0),
	_nextButton("Next"),
	_currentPage(2)
{
	showWindow->SignalChangeManagement().connect(sigc::mem_fun(*this, &ChaseWizard::onManagementChange));
	showWindow->SignalUpdateControllables().connect(sigc::mem_fun(*this, &ChaseWizard::fillFixturesList));
	
	initPage2();
	
	_nextButton.signal_clicked().connect(sigc::mem_fun(*this, &ChaseWizard::onNextClicked));
	_buttonBox.pack_start(_nextButton);
	_buttonBox.show_all();
	_mainBox.pack_end(_buttonBox);
	
	add(_mainBox);
	_mainBox.show();
}

void ChaseWizard::initPage2()
{
	_vBoxPage2.pack_start(_selectLabel);
	
	_fixturesListModel = Gtk::ListStore::create(_fixturesListColumns);
	_fixturesListView.set_model(_fixturesListModel);
	_fixturesListView.append_column("Fixture", _fixturesListColumns._title);
	_fixturesListView.append_column("Type", _fixturesListColumns._type);
	_fixturesListView.set_rubber_banding(true);
	_fixturesListView.get_selection()->set_mode(Gtk::SelectionMode::SELECTION_MULTIPLE);
	fillFixturesList();
	_fixturesScrolledWindow.add(_fixturesListView);
	_fixturesScrolledWindow.set_size_request(300, 400);
	_vBoxPage2.pack_start(_fixturesScrolledWindow);
	
	_mainBox.pack_start(_vBoxPage2, true, true);
	_vBoxPage2.show_all();
}

void ChaseWizard::initPage3()
{
	_colorHBox.pack_start(_colorLabel);
	_colorButton.signal_clicked().connect(sigc::mem_fun(*this, &ChaseWizard::onColorClicked));
	_colorHBox.pack_end(_colorButton);
	_colorArea.signal_draw().connect(sigc::mem_fun(*this, &ChaseWizard::onColorAreaDraw));
	_colorHBox.pack_end(_colorArea, true, true, 5);
	
	_vBoxPage3.pack_start(_colorHBox, true, false);
	
	_mainBox.pack_start(_vBoxPage3, true, true);
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
		case 2: {
		_selectedFixtures.clear();
		Glib::RefPtr<Gtk::TreeSelection> selection =
			_fixturesListView.get_selection();
		auto selected = selection->get_selected_rows();
		for(auto& row : selected)
		{
			_selectedFixtures.emplace_back((*_fixturesListModel->get_iter(row))[_fixturesListColumns._fixture]);
		}
		_mainBox.remove(_vBoxPage2);
		initPage3();
		_vBoxPage3.show_all();
		_currentPage = 3;
		} break;
		case 3:
		_mainBox.remove(_vBoxPage3);
		
		initPage2();
		_vBoxPage2.show_all();
		_currentPage = 2;
		makeChase();
		break;
	}
}

void ChaseWizard::onColorClicked()
{
	Gdk::RGBA color;
	color.set_red(_colorR);
	color.set_green(_colorG);
	color.set_blue(_colorB);
  color.set_alpha(1.0); //opaque
	
	Gtk::ColorChooserDialog dialog("Select color for running light");
	dialog.set_transient_for(*this);
	dialog.set_rgba(color);
	dialog.set_use_alpha(false);
	const int result = dialog.run();

	//Handle the response:
	if(result == Gtk::RESPONSE_OK)
	{
		//Store the chosen color:
		color = dialog.get_rgba();
		_colorR = color.get_red();
		_colorG = color.get_green();
		_colorB = color.get_blue();
		_colorArea.queue_draw();
	}
}

bool ChaseWizard::onColorAreaDraw(const Cairo::RefPtr<Cairo::Context>& cr)
{
	cr->set_source_rgb(_colorR, _colorG, _colorB);
  cr->paint();
  return true;
}

void ChaseWizard::makeChase()
{
	size_t sequenceLength = _selectedFixtures.size();
	Sequence& seq = _management->AddSequence();
	seq.SetName("Autochase");
	for(size_t chaseIndex=0; chaseIndex!=sequenceLength; ++chaseIndex)
	{
		PresetCollection& pc = _management->AddPresetCollection();
		pc.SetName("Autochase" + std::to_string(chaseIndex+1));
		unsigned
			red = unsigned(_colorR * double((1<<24)-1)),
			green = unsigned(_colorG * double((1<<24)-1)),
			blue = unsigned(_colorB * double((1<<24)-1));
		unsigned master = 0;
		if(red != 0 || green != 0 || blue != 0)
			master = (1<<24)-1;
		Fixture* f = _selectedFixtures[chaseIndex];
		for(const std::unique_ptr<FixtureFunction>& ff : f->Functions())
		{
			if(ff->Type() == FixtureFunction::RedIntensity && red != 0)
			{
				Controllable& c = _management->GetControllable(ff->Name());
				pc.AddPresetValue(*_management->GetPresetValue(c)).SetValue(red);
			}
			else if(ff->Type() == FixtureFunction::GreenIntensity && green != 0)
			{
				Controllable& c = _management->GetControllable(ff->Name());
				pc.AddPresetValue(*_management->GetPresetValue(c)).SetValue(green);
			}
			else if(ff->Type() == FixtureFunction::BlueIntensity && blue != 0)
			{
				Controllable& c = _management->GetControllable(ff->Name());
				pc.AddPresetValue(*_management->GetPresetValue(c)).SetValue(blue);
			}
			else if(ff->Type() == FixtureFunction::BlueIntensity && blue != 0)
			{
				Controllable& c = _management->GetControllable(ff->Name());
				pc.AddPresetValue(*_management->GetPresetValue(c)).SetValue(blue);
			}
			else if(ff->Type() == FixtureFunction::Brightness && master != 0)
			{
				Controllable& c = _management->GetControllable(ff->Name());
				pc.AddPresetValue(*_management->GetPresetValue(c)).SetValue(master);
			}
		}
		seq.AddPreset(&pc);
		_management->AddPreset(pc);
	}
	_showWindow->EmitUpdate();
}
