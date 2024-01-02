#include "fixturelistwindow.h"

#include "addfixturewindow.h"

#include "../eventtransmitter.h"
#include "../fixtureselection.h"

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "../../theatre/fixture.h"
#include "../../theatre/fixturecontrol.h"
#include "../../theatre/folder.h"
#include "../../theatre/management.h"
#include "../../theatre/theatre.h"

namespace glight::gui {

FixtureListWindow::FixtureListWindow(EventTransmitter &eventHub,
                                     theatre::Management &management,
                                     FixtureSelection &globalSelection)
    : _eventHub(eventHub),
      _management(management),
      _globalSelection(globalSelection),
      _newButton("New"),
      _removeButton("Remove"),
      _incChannelButton("+channel"),
      _decChannelButton("-channel"),
      _setChannelButton("Set..."),
      _upButton(),
      _downButton(),
      _reassignButton("Reassign") {
  set_title("Glight - fixtures");
  set_size_request(200, 400);

  _updateControllablesConnection =
      _eventHub.SignalUpdateControllables().connect(
          [&]() { FixtureListWindow::update(); });

  _globalSelectionConnection = _globalSelection.SignalChange().connect(
      [&]() { onGlobalSelectionChange(); });

  _fixturesListModel = Gtk::ListStore::create(_fixturesListColumns);

  _fixturesListView.get_selection()->signal_changed().connect(
      [&]() { onSelectionChanged(); });
  _fixturesListView.set_model(_fixturesListModel);
  _fixturesListView.append_column("Fixture", _fixturesListColumns._title);
  _fixturesListView.append_column("Type", _fixturesListColumns._type);
  _fixturesListView.append_column("Channels", _fixturesListColumns._channels);
  _fixturesListView.append_column("Symbol", _fixturesListColumns._symbol);
  fillFixturesList();
  _fixturesScrolledWindow.add(_fixturesListView);

  _fixturesScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  _mainBox.pack_start(_fixturesScrolledWindow);

  _buttonBox.set_homogeneous(true);

  _newButton.set_image_from_icon_name("document-new");
  _newButton.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onNewButtonClicked), false);
  _buttonBox.pack_start(_newButton);

  _removeButton.set_image_from_icon_name("edit-delete");
  _removeButton.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onRemoveButtonClicked));
  _buttonBox.pack_start(_removeButton);

  _decChannelButton.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onDecChannelButtonClicked));
  _buttonBox.pack_start(_decChannelButton);

  _incChannelButton.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onIncChannelButtonClicked));
  _buttonBox.pack_start(_incChannelButton);

  _setChannelButton.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onSetChannelButtonClicked));
  _buttonBox.pack_start(_setChannelButton);

  _upButton.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onUpClicked));
  _upButton.set_image_from_icon_name("go-up");
  _buttonBox.pack_start(_upButton);

  _downButton.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onDownClicked));
  _downButton.set_image_from_icon_name("go-down");
  _buttonBox.pack_start(_downButton);

  _reassignButton.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onReassignClicked));
  _buttonBox.pack_start(_reassignButton);

  _mainBox.pack_start(_buttonBox, false, false, 0);

  add(_mainBox);
  _mainBox.show_all();
}

FixtureListWindow::~FixtureListWindow() {
  _updateControllablesConnection.disconnect();
  _globalSelectionConnection.disconnect();
}

void FixtureListWindow::fillFixturesList() {
  _fixturesListModel->clear();

  std::lock_guard<std::mutex> lock(_management.Mutex());
  const std::vector<std::unique_ptr<theatre::Fixture>> &fixtures =
      _management.GetTheatre().Fixtures();
  for (const std::unique_ptr<theatre::Fixture> &fixture : fixtures) {
    Gtk::TreeModel::iterator iter = _fixturesListModel->append();
    const Gtk::TreeModel::Row &row = *iter;
    row[_fixturesListColumns._title] = fixture->Name();
    row[_fixturesListColumns._type] = fixture->Type().Name();
    row[_fixturesListColumns._channels] = getChannelString(*fixture);
    row[_fixturesListColumns._symbol] = fixture->Symbol().Name();
    row[_fixturesListColumns._fixture] = fixture.get();
  }
}

std::string FixtureListWindow::getChannelString(
    const theatre::Fixture &fixture) {
  std::vector<unsigned> channels = fixture.GetChannels();

  std::vector<unsigned>::const_iterator i = channels.begin();
  std::ostringstream s;
  // Note that DMX channels start counting at one on fixtures, but internally
  // are represented zero-indexed. Hence, add one.
  if (i != channels.end()) s << (*i + 1);
  ++i;
  for (; i != channels.end(); ++i) s << "," << (*i + 1);

  return s.str();
}

void FixtureListWindow::onNewButtonClicked() {
  add_fixture_window_ =
      std::make_unique<AddFixtureWindow>(&_eventHub, _management);
  add_fixture_window_->set_modal(true);
  add_fixture_window_->set_transient_for(*this);
  add_fixture_window_->show();
}

void FixtureListWindow::onRemoveButtonClicked() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _fixturesListView.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected) {
    theatre::Fixture *fixture = (*selected)[_fixturesListColumns._fixture];
    std::lock_guard<std::mutex> lock(_management.Mutex());
    _management.RemoveFixture(*fixture);
  }
  _eventHub.EmitUpdate();
}

void FixtureListWindow::onIncChannelButtonClicked() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _fixturesListView.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected) {
    theatre::Fixture *fixture = (*selected)[_fixturesListColumns._fixture];
    fixture->IncChannel();
    if (!fixture->IsVisible())
      fixture->SetSymbol(theatre::FixtureSymbol::Normal);
    updateFixture(fixture);
  }
}

void FixtureListWindow::onDecChannelButtonClicked() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _fixturesListView.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected) {
    theatre::Fixture *fixture = (*selected)[_fixturesListColumns._fixture];
    fixture->DecChannel();
    if (!fixture->IsVisible())
      fixture->SetSymbol(theatre::FixtureSymbol::Normal);
    updateFixture(fixture);
  }
}

void FixtureListWindow::onSetChannelButtonClicked() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _fixturesListView.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected) {
    theatre::Fixture *fixture = (*selected)[_fixturesListColumns._fixture];
    Gtk::MessageDialog dialog(*this, "Set DMX channel", false,
                              Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
    Gtk::Entry entry;
    entry.set_text(std::to_string(
        fixture->Functions().front()->MainChannel().Channel() + 1));
    dialog.get_message_area()->pack_start(entry, Gtk::PACK_SHRINK);
    dialog.get_message_area()->show_all_children();
    dialog.set_secondary_text(
        "Please enter the new DMX channel for this fixture");
    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) {
      std::string dmxChannel = entry.get_text();
      unsigned value = std::atoi(dmxChannel.c_str());
      if (value > 0 && value <= 512) {
        if (!fixture->IsVisible())
          fixture->SetSymbol(theatre::FixtureSymbol::Normal);
        const unsigned universe = 0;  // TODO
        fixture->SetChannel(theatre::DmxChannel(value - 1, universe));
        updateFixture(fixture);
      }
    }
  }
}

void FixtureListWindow::updateFixture(const theatre::Fixture *fixture) {
  Gtk::TreeModel::iterator iter = _fixturesListModel->children().begin();
  while (iter) {
    Gtk::TreeModel::Row row = *iter;
    if (fixture == (*iter)[_fixturesListColumns._fixture]) {
      row[_fixturesListColumns._title] = fixture->Name();
      row[_fixturesListColumns._type] = fixture->Type().Name();
      row[_fixturesListColumns._channels] = getChannelString(*fixture);
      row[_fixturesListColumns._symbol] = fixture->Symbol().Name();
      return;
    }
    ++iter;
  }
  throw std::runtime_error(
      "ConfigurationWindow::updateFixture(): Could not find fixture");
}

void FixtureListWindow::onSelectionChanged() {
  if (_recursionLock.IsFirst()) {
    RecursionLock::Token token(_recursionLock);
    Glib::RefPtr<Gtk::TreeSelection> selection =
        _fixturesListView.get_selection();
    Gtk::TreeModel::iterator selected = selection->get_selected();
    if (selected) {
      theatre::Fixture *fixture = (*selected)[_fixturesListColumns._fixture];
      _globalSelection.SetSelection(std::vector<theatre::Fixture *>{fixture});
    } else {
      _globalSelection.SetSelection(std::vector<theatre::Fixture *>());
    }
  }
}

void FixtureListWindow::onGlobalSelectionChange() {
  if (_recursionLock.IsFirst()) {
    RecursionLock::Token token(_recursionLock);
    _fixturesListView.get_selection()->unselect_all();
    if (!_globalSelection.Selection().empty()) {
      for (const auto &child : _fixturesListModel->children()) {
        if (child[_fixturesListColumns._fixture] ==
            _globalSelection.Selection().front()) {
          _fixturesListView.get_selection()->select(child);
          break;
        }
      }
    }
  }
}

void FixtureListWindow::onUpClicked() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _fixturesListView.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected) {
    theatre::Fixture *fixture = (*selected)[_fixturesListColumns._fixture];
    std::unique_lock<std::mutex> lock(_management.Mutex());
    theatre::Fixture *previous_fixture = nullptr;
    for (const std::unique_ptr<theatre::Fixture> &f :
         _management.GetTheatre().Fixtures()) {
      if (f.get() == fixture) {
        if (previous_fixture) {
          _management.GetTheatre().SwapFixturePositions(*previous_fixture,
                                                        *fixture);
        }
        break;
      }
      previous_fixture = f.get();
    }
    lock.unlock();
    _eventHub.EmitUpdate();
    _globalSelection.SetSelection(std::vector{fixture});
  }
}

void FixtureListWindow::onDownClicked() {
  Glib::RefPtr<Gtk::TreeSelection> selection =
      _fixturesListView.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected) {
    theatre::Fixture *fixture = (*selected)[_fixturesListColumns._fixture];
    std::unique_lock<std::mutex> lock(_management.Mutex());
    for (auto iterator = _management.GetTheatre().Fixtures().begin();
         iterator != _management.GetTheatre().Fixtures().end(); ++iterator) {
      if (iterator->get() == fixture) {
        ++iterator;
        if (iterator != _management.GetTheatre().Fixtures().end()) {
          _management.GetTheatre().SwapFixturePositions(**iterator, *fixture);
        }
        break;
      }
    }
    lock.unlock();
    _eventHub.EmitUpdate();
    _globalSelection.SetSelection(std::vector{fixture});
  }
}

void FixtureListWindow::onReassignClicked() {
  unsigned channel = 0;
  unsigned universe = 0;
  for (const std::unique_ptr<theatre::Fixture> &fixture :
       _management.GetTheatre().Fixtures()) {
    fixture->SetChannel(theatre::DmxChannel(channel, universe));
    const std::vector<unsigned> channels = fixture->GetChannels();
    for (unsigned ch : channels) {
      channel = std::max(channel, ch + 1);
    }
  }
  _eventHub.EmitUpdate();
}

}  // namespace glight::gui
