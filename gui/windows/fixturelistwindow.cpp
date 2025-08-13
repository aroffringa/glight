#include "fixturelistwindow.h"

#include "addfixturewindow.h"

#include "gui/eventtransmitter.h"
#include "gui/fixtureselection.h"
#include "gui/instance.h"

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "theatre/fixture.h"
#include "theatre/fixturecontrol.h"
#include "theatre/fixturetype.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/theatre.h"

namespace glight::gui::windows {

FixtureListWindow::FixtureListWindow() {
  set_title("Glight - fixtures");
  set_size_request(200, 400);

  update_controllables_connection_ =
      Instance::Events().SignalUpdateControllables().connect(
          [&]() { FixtureListWindow::update(); });

  global_selection_connection_ = Instance::Selection().SignalChange().connect(
      [&]() { onGlobalSelectionChange(); });

  fixtures_list_model_ = Gtk::ListStore::create(fixtures_list_columns_);

  fixtures_list_view_.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
  fixtures_list_view_.get_selection()->signal_changed().connect(
      [&]() { onSelectionChanged(); });
  fixtures_list_view_.set_model(fixtures_list_model_);
  fixtures_list_view_.append_column("Fixture", fixtures_list_columns_.title_);
  fixtures_list_view_.append_column("Type", fixtures_list_columns_.type_);
  fixtures_list_view_.append_column("Channels",
                                    fixtures_list_columns_.channels_);
  fixtures_list_view_.append_column("Universe",
                                    fixtures_list_columns_.universe_);
  fixtures_list_view_.append_column("Symbol", fixtures_list_columns_.symbol_);
  fixtures_list_view_.set_rubber_banding(true);
  fillFixturesList();
  fixtures_scrolled_window_.add(fixtures_list_view_);

  fixtures_scrolled_window_.set_policy(Gtk::POLICY_NEVER,
                                       Gtk::POLICY_AUTOMATIC);
  fixtures_scrolled_window_.set_hexpand(true);
  fixtures_scrolled_window_.set_vexpand(true);

  grid_.attach(fixtures_scrolled_window_, 0, 0, 1, 8);
  grid_.set_hexpand(true);
  grid_.set_vexpand(true);

  new_button_.set_image_from_icon_name("document-new");
  new_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onNewButtonClicked), false);
  grid_.attach(new_button_, 1, 0, 2, 1);

  remove_button_.set_image_from_icon_name("edit-delete");
  remove_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onRemoveButtonClicked));
  grid_.attach(remove_button_, 1, 1, 2, 1);

  dec_channel_button_.signal_clicked().connect(
      [&]() { IncreaseChannelOrUniverse<-1, 0>(); });
  grid_.attach(dec_channel_button_, 1, 2, 1, 1);

  inc_channel_button_.signal_clicked().connect(
      [&]() { IncreaseChannelOrUniverse<1, 0>(); });
  grid_.attach(inc_channel_button_, 1, 3, 1, 1);

  dec_universe_button_.signal_clicked().connect(
      [&]() { IncreaseChannelOrUniverse<0, -1>(); });
  grid_.attach(dec_universe_button_, 2, 2, 1, 1);

  inc_universe_button_.signal_clicked().connect(
      [&]() { IncreaseChannelOrUniverse<0, 1>(); });
  grid_.attach(inc_universe_button_, 2, 3, 1, 1);

  set_channel_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onSetChannelButtonClicked));
  grid_.attach(set_channel_button_, 1, 4, 2, 1);

  up_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onUpClicked));
  up_button_.set_image_from_icon_name("go-up");
  grid_.attach(up_button_, 1, 5, 2, 1);

  down_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onDownClicked));
  down_button_.set_image_from_icon_name("go-down");
  grid_.attach(down_button_, 1, 6, 2, 1);

  reassign_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureListWindow::onReassignClicked));
  grid_.attach(reassign_button_, 1, 7, 2, 1);

  add(grid_);
  grid_.show_all();
}

FixtureListWindow::~FixtureListWindow() = default;

void FixtureListWindow::fillFixturesList() {
  fixtures_list_model_->clear();

  std::lock_guard<std::mutex> lock(Instance::Management().Mutex());
  const std::vector<system::TrackablePtr<theatre::Fixture>> &fixtures =
      Instance::Management().GetTheatre().Fixtures();
  for (const system::TrackablePtr<theatre::Fixture> &fixture : fixtures) {
    Gtk::TreeModel::iterator iter = fixtures_list_model_->append();
    const Gtk::TreeModel::Row &row = *iter;
    row[fixtures_list_columns_.title_] = fixture->Name();
    row[fixtures_list_columns_.type_] = fixture->Mode().Name();
    row[fixtures_list_columns_.universe_] = fixture->GetUniverse();
    row[fixtures_list_columns_.channels_] = getChannelString(*fixture);
    row[fixtures_list_columns_.symbol_] = fixture->Symbol().Name();
    row[fixtures_list_columns_.fixture_] = fixture.GetObserver();
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
  add_fixture_window_ = std::make_unique<AddFixtureWindow>(
      &Instance::Events(), Instance::Management());
  add_fixture_window_->set_modal(true);
  add_fixture_window_->set_transient_for(*this);
  add_fixture_window_->show();
}

std::vector<system::ObservingPtr<theatre::Fixture>>
FixtureListWindow::GetSelection() const {
  Glib::RefPtr<const Gtk::TreeSelection> selection =
      fixtures_list_view_.get_selection();
  std::vector<Gtk::TreeModel::Path> rows = selection->get_selected_rows();
  std::vector<system::ObservingPtr<theatre::Fixture>> fixtures;
  for (const Gtk::TreeModel::Path &path : rows) {
    const Gtk::TreeModel::iterator iter = fixtures_list_model_->get_iter(path);
    if (iter) {
      const system::ObservingPtr<theatre::Fixture> &fixture =
          (*iter)[fixtures_list_columns_.fixture_];
      fixtures.emplace_back(fixture);
    }
  }
  return fixtures;
}

void FixtureListWindow::onRemoveButtonClicked() {
  std::unique_lock<std::mutex> lock(Instance::Management().Mutex());
  const std::vector<system::ObservingPtr<theatre::Fixture>> selection =
      GetSelection();
  for (const system::ObservingPtr<theatre::Fixture> &fixture : selection) {
    Instance::Management().RemoveFixture(*fixture);
  }
  lock.unlock();
  Instance::Selection().UpdateAfterDelete();
  Instance::Events().EmitUpdate();
}

void FixtureListWindow::onSetChannelButtonClicked() {
  const std::vector<system::ObservingPtr<theatre::Fixture>> selection =
      GetSelection();
  if (selection.size() == 1) {
    const system::ObservingPtr<theatre::Fixture> &fixture = selection[0];
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
        std::unique_lock<std::mutex> lock(Instance::Management().Mutex());
        if (!fixture->IsVisible())
          fixture->SetSymbol(theatre::FixtureSymbol::Normal);
        const unsigned universe = 0;  // TODO
        fixture->SetChannel(theatre::DmxChannel(value - 1, universe));
        updateFixture(fixture.Get());
      }
    }
  }
}

void FixtureListWindow::updateFixture(const theatre::Fixture *fixture) {
  for (Gtk::TreeModel::iterator iter = fixtures_list_model_->children().begin();
       iter; ++iter) {
    Gtk::TreeModel::Row row = *iter;
    if (fixture == (*iter)[fixtures_list_columns_.fixture_]) {
      row[fixtures_list_columns_.title_] = fixture->Name();
      row[fixtures_list_columns_.type_] = fixture->Mode().Name();
      row[fixtures_list_columns_.channels_] = getChannelString(*fixture);
      row[fixtures_list_columns_.universe_] = fixture->GetUniverse();
      row[fixtures_list_columns_.symbol_] = fixture->Symbol().Name();
      return;
    }
  }
  throw std::runtime_error(
      "ConfigurationWindow::updateFixture(): Could not find fixture");
}

void FixtureListWindow::onSelectionChanged() {
  if (recursion_lock_.IsFirst()) {
    RecursionLock::Token token(recursion_lock_);
    Instance::Selection().SetSelection(GetSelection());
  }
}

void FixtureListWindow::onGlobalSelectionChange() {
  if (recursion_lock_.IsFirst()) {
    RecursionLock::Token token(recursion_lock_);
    fixtures_list_view_.get_selection()->unselect_all();
    const std::vector<system::ObservingPtr<theatre::Fixture>> &new_selection =
        Instance::Selection().Selection();
    for (const auto &child : fixtures_list_model_->children()) {
      const system::ObservingPtr<theatre::Fixture> &fixture =
          child[fixtures_list_columns_.fixture_];
      auto iter =
          std::find(new_selection.begin(), new_selection.end(), fixture);
      if (iter != new_selection.end()) {
        fixtures_list_view_.get_selection()->select(child);
      }
    }
  }
}

void FixtureListWindow::onUpClicked() {
  std::unique_lock<std::mutex> lock(Instance::Management().Mutex());
  const std::vector<system::ObservingPtr<theatre::Fixture>> selection =
      GetSelection();
  for (const system::ObservingPtr<theatre::Fixture> &fixture : selection) {
    theatre::Fixture *previous_fixture = nullptr;
    for (const system::TrackablePtr<theatre::Fixture> &f :
         Instance::Management().GetTheatre().Fixtures()) {
      if (f.Get() == fixture) {
        if (previous_fixture) {
          Instance::Management().GetTheatre().SwapFixturePositions(
              *previous_fixture, *fixture);
        }
        break;
      }
      previous_fixture = f.Get();
    }
  }
  lock.unlock();
  Instance::Events().EmitUpdate();
  Instance::Selection().SetSelection(selection);
}

void FixtureListWindow::onDownClicked() {
  std::unique_lock<std::mutex> lock(Instance::Management().Mutex());
  const std::vector<system::ObservingPtr<theatre::Fixture>> selection =
      GetSelection();
  for (const system::ObservingPtr<theatre::Fixture> &fixture : selection) {
    for (auto iterator = Instance::Management().GetTheatre().Fixtures().begin();
         iterator != Instance::Management().GetTheatre().Fixtures().end();
         ++iterator) {
      if (iterator->Get() == fixture) {
        ++iterator;
        if (iterator != Instance::Management().GetTheatre().Fixtures().end()) {
          Instance::Management().GetTheatre().SwapFixturePositions(**iterator,
                                                                   *fixture);
        }
        break;
      }
    }
  }
  lock.unlock();
  Instance::Events().EmitUpdate();
  Instance::Selection().SetSelection(selection);
}

void FixtureListWindow::onReassignClicked() {
  unsigned channel = 0;
  unsigned universe = 0;
  for (const system::TrackablePtr<theatre::Fixture> &fixture :
       Instance::Management().GetTheatre().Fixtures()) {
    fixture->SetChannel(theatre::DmxChannel(channel, universe));
    const std::vector<unsigned> channels = fixture->GetChannels();
    for (unsigned ch : channels) {
      channel = std::max(channel, ch + 1);
    }
  }
  Instance::Events().EmitUpdate();
}

template <int ChannelIncrease, int UniverseIncrease>
void FixtureListWindow::IncreaseChannelOrUniverse() {
  std::unique_lock<std::mutex> lock(Instance::Management().Mutex());
  const std::vector<system::ObservingPtr<theatre::Fixture>> selection =
      GetSelection();
  for (const system::ObservingPtr<theatre::Fixture> &fixture : selection) {
    if constexpr (ChannelIncrease > 0) {
      for (int i = 0; i != ChannelIncrease; ++i) fixture->IncChannel();
    } else if constexpr (ChannelIncrease < 0) {
      for (int i = 0; i != -ChannelIncrease; ++i) fixture->DecChannel();
    }
    if constexpr (ChannelIncrease != 0) {
      if (!fixture->IsVisible())
        fixture->SetSymbol(theatre::FixtureSymbol::Normal);
    }
    const int universe = fixture->GetUniverse();
    if constexpr (UniverseIncrease != 0) {
      if (universe + UniverseIncrease >= 0) {
        fixture->SetUniverse(universe + UniverseIncrease);
      }
    }
    updateFixture(fixture.Get());
  }
}

}  // namespace glight::gui::windows
