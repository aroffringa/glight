#include "addfixturewindow.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "theatre/fixturecontrol.h"
#include "theatre/fixturemode.h"
#include "theatre/fixturemodefunction.h"
#include "theatre/fixturetype.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/theatre.h"

#include "theatre/filters/automasterfilter.h"
#include "theatre/filters/colortemperaturefilter.h"
#include "theatre/filters/monochromefilter.h"
#include "theatre/filters/rgbfilter.h"

namespace glight::gui::windows {

using theatre::FixtureMode;
using theatre::FixtureModeFunction;
using theatre::FixtureType;

AddFixtureWindow::AddFixtureWindow() {
  const std::vector<theatre::StockFixture> stock_fixtures =
      theatre::GetStockFixtureList();
  for (theatre::StockFixture stock_fixture : stock_fixtures) {
    stock_list_.emplace(ToString(stock_fixture), stock_fixture);
  }

  grid_.set_row_spacing(5);
  grid_.set_column_spacing(2);

  Gtk::RadioButton::Group group;
  stock_or_project_box_.pack_start(stock_button_);
  stock_button_.set_group(group);
  stock_button_.set_hexpand(true);
  stock_button_.signal_toggled().connect([&]() { onStockProjectToggled(); });

  stock_or_project_box_.pack_start(project_button_);
  project_button_.set_group(group);
  project_button_.set_hexpand(true);
  project_button_.signal_toggled().connect([&]() { onStockProjectToggled(); });

  grid_.attach(stock_or_project_box_, 0, 0, 2, 1);
  stock_or_project_box_.set_halign(Gtk::ALIGN_CENTER);
  stock_or_project_box_.set_hexpand(true);

  grid_.attach(type_label_, 0, 1, 1, 1);

  type_model_ = Gtk::ListStore::create(type_columns_);
  channel_mode_model_ = Gtk::ListStore::create(mode_columns_);

  type_combo_.set_model(type_model_);
  type_combo_.pack_start(type_columns_.type_str_);
  type_combo_.signal_changed().connect([&]() { updateModes(); });
  grid_.attach(type_combo_, 1, 1, 3, 1);

  grid_.attach(channel_mode_label_, 0, 2, 1, 1);
  channel_mode_combo_.set_model(channel_mode_model_);
  channel_mode_combo_.pack_start(mode_columns_.mode_str_);
  channel_mode_combo_.signal_changed().connect([&]() { updateFilters(); });
  grid_.attach(channel_mode_combo_, 1, 2, 3, 1);

  fillStock();

  grid_.attach(count_label_, 0, 3, 1, 1);
  count_entry_.set_text("1"), grid_.attach(count_entry_, 1, 3, 1, 1);
  decrease_count_button_.signal_clicked().connect([&]() { onDecCount(); });
  grid_.attach(decrease_count_button_, 2, 3, 1, 1);
  increase_count_button_.signal_clicked().connect([&]() { onIncCount(); });
  grid_.attach(increase_count_button_, 3, 3, 1, 1);

  filters_box_.pack_start(auto_master_cb_);
  filters_box_.pack_start(rgb_cb_);
  filters_box_.pack_start(monochrome_cb_);
  filters_box_.pack_start(temperature_cb_);
  updateFilters();

  filters_frame_.add(filters_box_);
  grid_.attach(filters_frame_, 0, 4, 4, 1);

  button_box_.set_homogeneous(true);

  cancel_button_.signal_clicked().connect([&]() { onCancel(); });
  button_box_.pack_start(cancel_button_);
  add_button_.signal_clicked().connect([&]() { onAdd(); });
  button_box_.pack_end(add_button_);
  grid_.attach(button_box_, 0, 5, 4, 1);

  add(grid_);
  grid_.set_hexpand(true);
  show_all_children();
}

void AddFixtureWindow::onStockProjectToggled() {
  if (stock_button_.get_active())
    fillStock();
  else
    fillFromProject();
}

system::ObservingPtr<theatre::FixtureType> AddFixtureWindow::GetSelectedType(
    system::TrackablePtr<theatre::FixtureType> &stock_type) {
  Gtk::TreeModel::const_iterator selected_type = type_combo_.get_active();
  system::ObservingPtr<theatre::FixtureType> type =
      (*selected_type)[type_columns_.type_];
  if (!type) {
    stock_type = system::MakeTrackable<theatre::FixtureType>(
        (*selected_type)[type_columns_.stock_fixture_]);
    type = stock_type.GetObserver();
  }
  return type;
}

void AddFixtureWindow::updateModes() {
  Gtk::TreeModel::const_iterator selected_type = type_combo_.get_active();
  if (selected_type) {
    channel_mode_model_->clear();
    system::TrackablePtr<theatre::FixtureType> stock_type;
    system::ObservingPtr<theatre::FixtureType> type =
        GetSelectedType(stock_type);
    const std::vector<FixtureMode> &modes = type->Modes();
    for (size_t index = 0; index != modes.size(); ++index) {
      Gtk::TreeModel::iterator iter = channel_mode_model_->append();
      (*iter)[mode_columns_.mode_str_] = modes[index].Name();
      (*iter)[mode_columns_.mode_index_] = index;
      if (index == 0) {
        channel_mode_combo_.set_active(iter);
      }
    }
  }
}

void AddFixtureWindow::updateFilters() {
  Gtk::TreeModel::const_iterator selected_type = type_combo_.get_active();
  Gtk::TreeModel::const_iterator selected_mode =
      channel_mode_combo_.get_active();
  bool enable_master = false;
  bool enable_color = false;
  bool enable_monochrome = false;
  bool has_color = false;
  bool has_temperature = false;
  if (selected_type && selected_mode) {
    system::TrackablePtr<theatre::FixtureType> stock_type;
    system::ObservingPtr<theatre::FixtureType> type =
        GetSelectedType(stock_type);
    const size_t mode_index = (*selected_mode)[mode_columns_.mode_index_];
    const FixtureMode &mode = type->Modes()[mode_index];
    for (const FixtureModeFunction &function : mode.Functions()) {
      if (function.Type() == theatre::FunctionType::Master)
        enable_master = true;
      else if (IsColor(function.Type())) {
        has_color = true;
        if (!IsRgb(function.Type()))
          enable_color = true;
        else if (function.Type() != theatre::FunctionType::White)
          enable_monochrome = true;
      } else if (function.Type() == theatre::FunctionType::ColorTemperature) {
        has_temperature = true;
      } else if (function.Type() == theatre::FunctionType::ColorMacro ||
                 function.Type() == theatre::FunctionType::ColorWheel) {
        enable_color = true;
      }
    }
  }
  has_temperature = has_temperature && enable_master;
  if (!has_color || has_temperature) {
    // Don't enable master if there are no colour channels, because the filter
    // needs the colour to deduce the master channel
    enable_master = false;
  }
  auto_master_cb_.set_active(enable_master);
  auto_master_cb_.set_sensitive(enable_master);
  temperature_cb_.set_active(has_temperature && !has_color);
  temperature_cb_.set_sensitive(has_temperature);
  rgb_cb_.set_active(enable_color);
  rgb_cb_.set_sensitive(enable_color);
  monochrome_cb_.set_sensitive(enable_monochrome);
}

void AddFixtureWindow::fillStock() {
  type_model_->clear();
  for (const std::pair<const std::string, theatre::StockFixture> &item :
       stock_list_) {
    Gtk::TreeModel::iterator iter = type_model_->append();
    (*iter)[type_columns_.type_str_] = item.first;
    (*iter)[type_columns_.stock_fixture_] = item.second;
    if (item.first == ToString(theatre::StockFixture::Rgb3Ch)) {
      type_combo_.set_active(iter);
    }
  }
  if (!type_combo_.get_active()) type_combo_.set_active(0);
  add_button_.set_sensitive(true);
}

void AddFixtureWindow::fillFromProject() {
  type_model_->clear();
  theatre::Management &management = Instance::Management();
  const std::vector<system::TrackablePtr<FixtureType>> &types =
      management.GetTheatre().FixtureTypes();
  for (const system::TrackablePtr<FixtureType> &type : types) {
    Gtk::TreeModel::iterator iter = type_model_->append();
    (*iter)[type_columns_.type_str_] = type->Name();
    (*iter)[type_columns_.type_] = type.GetObserver();
  }
  if (types.empty())
    add_button_.set_sensitive(false);
  else
    type_combo_.set_active(0);
}

void AddFixtureWindow::onAdd() {
  Gtk::TreeModel::const_iterator iter = type_combo_.get_active();
  const int count = std::atoi(count_entry_.get_text().c_str());
  if (iter && count > 0) {
    theatre::Management &management = Instance::Management();
    std::unique_lock<std::mutex> lock(management.Mutex());

    system::ObservingPtr<FixtureType> type = (*iter)[type_columns_.type_];
    if (!type) {
      const theatre::StockFixture stock_fixture =
          (*iter)[type_columns_.stock_fixture_];
      FixtureType *project_type = dynamic_cast<FixtureType *>(
          management.RootFolder().GetChildIfExists(ToString(stock_fixture)));
      if (project_type) {
        type = management.GetTheatre().GetFixtureTypePtr(*project_type);
      } else {
        type = management.GetTheatre().AddFixtureTypePtr(stock_fixture);
        management.RootFolder().Add(type);
      }
    }
    // TODO make mode selectable
    const FixtureMode &mode = type->Modes().front();

    for (size_t fixIter = 0; fixIter != static_cast<size_t>(count); ++fixIter) {
      const theatre::Coordinate3D position =
          management.GetTheatre().GetFreePosition();
      theatre::Fixture &fixture = *management.GetTheatre().AddFixture(mode);
      theatre::DmxChannel channel(
          fixture.GetFirstChannel().Channel(),
          management.GetUniverses().FirstOutputUniverse());
      fixture.SetChannel(channel);
      fixture.GetPosition() = position;

      theatre::FixtureControl &control =
          static_cast<theatre::FixtureControl &>(*management.AddFixtureControl(
              fixture, management.RootFolder() /* TODO */));
      if (temperature_cb_.get_active()) {
        control.AddFilter(std::make_unique<theatre::ColorTemperatureFilter>());
      } else {
        if (auto_master_cb_.get_active()) {
          control.AddFilter(std::make_unique<theatre::AutoMasterFilter>());
        }
        if (rgb_cb_.get_active()) {
          control.AddFilter(std::make_unique<theatre::RgbFilter>());
        }
      }
      if (monochrome_cb_.get_active()) {
        control.AddFilter(std::make_unique<theatre::MonochromeFilter>());
      }
      for (size_t i = 0; i != control.NInputs(); ++i) {
        management.AddSourceValue(control, i);
      }
    }

    lock.unlock();
    Instance::Events().EmitUpdate();
    hide();
  }
}

void AddFixtureWindow::onDecCount() {
  int count = std::atoi(count_entry_.get_text().c_str());
  if (count > 1)
    count--;
  else
    count = 1;
  count_entry_.set_text(std::to_string(count));
}

void AddFixtureWindow::onIncCount() {
  int count = std::atoi(count_entry_.get_text().c_str());
  count++;
  count_entry_.set_text(std::to_string(count));
}

}  // namespace glight::gui::windows
