#include "configurationwindow.h"
#include "showwindow.h"

#include <boost/thread/locks.hpp>

#include <gtkmm/stock.h>
#include <gtkmm/messagedialog.h>

#include "../libtheatre/fixture.h"
#include "../libtheatre/fixturecontrol.h"
#include "../libtheatre/management.h"
#include "../libtheatre/theatre.h"

ConfigurationWindow::ConfigurationWindow(ShowWindow* showWindow) :
	_showWindow(showWindow),
	_management(&showWindow->GetManagement()),
	_newButton("New"),
	_removeButton("Remove"),
	_incChannelButton("+channel"),
	_decChannelButton("-channel"),
	_setChannelButton("Set...")
{
	set_title("Glight - configuration");
	
	showWindow->SignalChangeManagement().connect(sigc::mem_fun(*this, &ConfigurationWindow::onChangeManagement));
	showWindow->SignalUpdateControllables().connect(sigc::mem_fun(*this, &ConfigurationWindow::update));

	_fixturesListModel =
    Gtk::ListStore::create(_fixturesListColumns);

	_fixturesListView.set_model(_fixturesListModel);
	_fixturesListView.append_column("Fixture", _fixturesListColumns._title);
	_fixturesListView.append_column("Type", _fixturesListColumns._type);
	_fixturesListView.append_column("Channels", _fixturesListColumns._channels);
	fillFixturesList();
	_fixturesScrolledWindow.add(_fixturesListView);

	_fixturesScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_mainBox.pack_start(_fixturesScrolledWindow);

	_newButton.set_image_from_icon_name("document-new");
	_newButton.set_events(Gdk::BUTTON_PRESS_MASK);
	_newButton.signal_button_press_event().connect(sigc::mem_fun(*this, &ConfigurationWindow::onNewButtonClicked), false);
	_buttonBox.pack_start(_newButton);

	_removeButton.set_image_from_icon_name("edit-delete");
	_removeButton.signal_clicked().connect(sigc::mem_fun(*this, &ConfigurationWindow::onRemoveButtonClicked));
	_buttonBox.pack_start(_removeButton);

	_decChannelButton.signal_clicked().connect(sigc::mem_fun(*this, &ConfigurationWindow::onDecChannelButtonClicked));
	_buttonBox.pack_start(_decChannelButton);

	_incChannelButton.signal_clicked().connect(sigc::mem_fun(*this, &ConfigurationWindow::onIncChannelButtonClicked));
	_buttonBox.pack_start(_incChannelButton);

	_setChannelButton.signal_clicked().connect(sigc::mem_fun(*this, &ConfigurationWindow::onSetChannelButtonClicked));
	_buttonBox.pack_start(_setChannelButton);

	_mainBox.pack_start(_buttonBox, false, false, 0);

	add(_mainBox);
	_mainBox.show_all();
}

void ConfigurationWindow::fillFixturesList()
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
		row[_fixturesListColumns._channels] = getChannelString(*fixture);
		row[_fixturesListColumns._fixture] = fixture.get();
	}
}

std::string ConfigurationWindow::getChannelString(const class Fixture& fixture)
{
	std::vector<unsigned> channels = fixture.GetChannels();
	
	std::vector<unsigned>::const_iterator i=channels.begin();
	std::ostringstream s;
	// Note that DMX channels start counting at one on fixtures, but internally are represented
	// zero-indexed. Hence, add one.
	if(i != channels.end())
		s << (*i+1);
	++i;
	for(;i!=channels.end();++i)
		s << "," << (*i+1);

	return s.str();
}

bool ConfigurationWindow::onNewButtonClicked(GdkEventButton* event)
{
	if(event->button == 1)
	{
		_popupMenuItems.clear();
	
		_popupMenu.reset(new Gtk::Menu());
	
		const std::vector<enum FixtureType::FixtureClass> &classes = FixtureType::GetClassList();
		for(enum FixtureType::FixtureClass fc : classes)
		{
			std::unique_ptr<Gtk::MenuItem> mi(new Gtk::MenuItem(FixtureType::ClassName(fc)));
			mi->signal_activate().connect(sigc::bind<enum FixtureType::FixtureClass>( 
			sigc::mem_fun(*this, &ConfigurationWindow::onMenuItemClicked), fc));
			_popupMenu->append(*mi);
			_popupMenuItems.emplace_back(std::move(mi));
		}
		_popupMenu->show_all_children();
		_popupMenu->popup(event->button, event->time);
		return true;
	}
	return false;
}

void ConfigurationWindow::onRemoveButtonClicked()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
    _fixturesListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
	{
		Fixture* fixture = (*selected)[_fixturesListColumns._fixture];
		std::lock_guard<std::mutex> lock(_management->Mutex());
		_management->RemoveFixture(*fixture);
	}	
	_showWindow->EmitUpdate();
}

#include <iostream>
void ConfigurationWindow::onMenuItemClicked(enum FixtureType::FixtureClass cl)
{
	std::unique_lock<std::mutex> lock(_management->Mutex());
	Position position = _management->Theatre().GetFreePosition();
	FixtureType &type = _management->Theatre().AddFixtureType(cl);
	Fixture &fixture = _management->Theatre().AddFixture(type);
	fixture.Position() = position;
	std::cout << position.X() << " " << position.Y() << '\n';
	
	const std::vector<std::unique_ptr<FixtureFunction>>& functions = fixture.Functions();

	int number = 1;
	FixtureControl& control = _management->AddFixtureControl(fixture, _management->RootFolder() /* TODO */);
	for(size_t i=0; i!=functions.size(); ++i)
	{
		//std::stringstream funcName;
		//funcName << fixture.Name() << number;
		_management->AddPreset(control, i);
		++number;
	}
	lock.unlock();

	_showWindow->EmitUpdate();
}

void ConfigurationWindow::onIncChannelButtonClicked()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
    _fixturesListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
	{
		Fixture* fixture = (*selected)[_fixturesListColumns._fixture];
		fixture->IncChannel();
		updateFixture(fixture);
	}
}

void ConfigurationWindow::onDecChannelButtonClicked()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
    _fixturesListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
	{
		Fixture* fixture = (*selected)[_fixturesListColumns._fixture];
		fixture->DecChannel();
		updateFixture(fixture);
	}
}

void ConfigurationWindow::onSetChannelButtonClicked()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
    _fixturesListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
	{
		Fixture* fixture = (*selected)[_fixturesListColumns._fixture];
		Gtk::MessageDialog dialog(*this, "Set DMX channel",
			false, Gtk::MESSAGE_QUESTION,
			Gtk::BUTTONS_OK_CANCEL);
		Gtk::Entry entry;
		entry.set_text(std::to_string(fixture->Functions().front()->FirstChannel().Channel()+1));
		dialog.get_vbox()->pack_start(entry, Gtk::PACK_SHRINK);
		dialog.get_vbox()->show_all_children();
		dialog.set_secondary_text(
			"Please enter the new DMX channel for this fixture");
		int result = dialog.run();
		if(result == Gtk::RESPONSE_OK)
		{
			std::string dmxChannel = entry.get_text();
			unsigned value = std::atoi(dmxChannel.c_str());
			if(value > 0 && value <= 512)
			{
				fixture->SetChannel(value-1);
				updateFixture(fixture);
			}
		}
	}
}

void ConfigurationWindow::updateFixture(const Fixture *fixture)
{
	Gtk::TreeModel::iterator iter = _fixturesListModel->children().begin();
	while(iter)
	{
		Gtk::TreeModel::Row row = *iter;
		if(fixture == (*iter)[_fixturesListColumns._fixture])
		{
			row[_fixturesListColumns._title] = fixture->Name();
			row[_fixturesListColumns._type] = fixture->Type().Name();
			row[_fixturesListColumns._channels] = getChannelString(*fixture);
			return;
		}
		++iter;
	}
	throw std::runtime_error("ConfigurationWindow::updateFixture(): Could not find fixture");
}
