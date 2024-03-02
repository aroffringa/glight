#include "addfixturewindow.h"

#include "gui/eventtransmitter.h"

#include "theatre/fixturecontrol.h"
#include "theatre/fixturetype.h"
#include "theatre/fixturetypefunction.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/theatre.h"

#include "theatre/filters/automasterfilter.h"
#include "theatre/filters/colortemperaturefilter.h"
#include "theatre/filters/monochromefilter.h"
#include "theatre/filters/rgbfilter.h"

namespace glight::gui {

using theatre::FixtureType;
using theatre::FixtureTypeFunction;

AddFixtureWindow::AddFixtureWindow(EventTransmitter *eventHub,
                                   theatre::Management &management)
    : _eventHub(*eventHub),
      _management(&management),
      stock_list_(theatre::FixtureType::GetStockTypes()) {
  _grid.set_row_spacing(5);
  _grid.set_column_spacing(2);

  Gtk::RadioButton::Group group;
  stock_or_project_box_.pack_start(stock_button_);
  stock_button_.set_group(group);
  stock_button_.set_hexpand(true);
  stock_button_.signal_toggled().connect([&]() { onStockProjectToggled(); });

  stock_or_project_box_.pack_start(project_button_);
  project_button_.set_group(group);
  project_button_.set_hexpand(true);
  project_button_.signal_toggled().connect([&]() { onStockProjectToggled(); });

  _grid.attach(stock_or_project_box_, 0, 0, 2, 1);
  stock_or_project_box_.set_halign(Gtk::ALIGN_CENTER);
  stock_or_project_box_.set_hexpand(true);

  _grid.attach(_typeLabel, 0, 1, 1, 1);

  type_model_ = Gtk::ListStore::create(type_columns_);
  _typeCombo.set_model(type_model_);
  _typeCombo.pack_start(type_columns_.type_str_);
  _typeCombo.signal_changed().connect([&]() { updateFilters(); });
  fillStock();
  _grid.attach(_typeCombo, 1, 1, 3, 1);

  _grid.attach(_countLabel, 0, 2, 1, 1);
  _countEntry.set_text("1"), _grid.attach(_countEntry, 1, 2, 1, 1);
  _decCountButton.signal_clicked().connect([&]() { onDecCount(); });
  _grid.attach(_decCountButton, 2, 2, 1, 1);
  _incCountButton.signal_clicked().connect([&]() { onIncCount(); });
  _grid.attach(_incCountButton, 3, 2, 1, 1);

  filters_box_.pack_start(auto_master_cb_);
  filters_box_.pack_start(rgb_cb_);
  filters_box_.pack_start(monochrome_cb_);
  filters_box_.pack_start(temperature_cb_);
  updateFilters();

  filters_frame_.add(filters_box_);
  _grid.attach(filters_frame_, 0, 3, 4, 1);

  _buttonBox.set_homogeneous(true);

  _cancelButton.signal_clicked().connect([&]() { onCancel(); });
  _buttonBox.pack_start(_cancelButton);
  _addButton.signal_clicked().connect([&]() { onAdd(); });
  _buttonBox.pack_end(_addButton);
  _grid.attach(_buttonBox, 0, 4, 4, 1);

  add(_grid);
  _grid.set_hexpand(true);
  show_all_children();
}

void AddFixtureWindow::onStockProjectToggled() {
  if (stock_button_.get_active())
    fillStock();
  else
    fillFromProject();
}

void AddFixtureWindow::updateFilters() {
  Gtk::TreeModel::const_iterator selected = _typeCombo.get_active();
  bool enable_master = false;
  bool enable_color = false;
  bool enable_monochrome = false;
  bool has_color = false;
  bool has_temperature = false;
  if (selected) {
    const FixtureType *type = (*selected)[type_columns_.type_];
    for (const FixtureTypeFunction &function : type->Functions()) {
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
  for (const std::pair<const std::string, FixtureType> &item : stock_list_) {
    Gtk::TreeModel::iterator iter = type_model_->append();
    (*iter)[type_columns_.type_str_] = item.first;
    (*iter)[type_columns_.type_] = &item.second;
    if (item.first == FixtureType::StockName(theatre::StockFixture::Rgb3Ch)) {
      _typeCombo.set_active(iter);
    }
  }
  if (!_typeCombo.get_active()) _typeCombo.set_active(0);
  _addButton.set_sensitive(true);
}

void AddFixtureWindow::fillFromProject() {
  type_model_->clear();
  const std::vector<std::unique_ptr<FixtureType>> &types =
      _management->GetTheatre().FixtureTypes();
  for (const std::unique_ptr<FixtureType> &type : types) {
    Gtk::TreeModel::iterator iter = type_model_->append();
    (*iter)[type_columns_.type_str_] = type->Name();
    (*iter)[type_columns_.type_] = type.get();
  }
  if (types.empty())
    _addButton.set_sensitive(false);
  else
    _typeCombo.set_active(0);
}

void AddFixtureWindow::onAdd() {
  Gtk::TreeModel::const_iterator iter = _typeCombo.get_active();
  const int count = std::atoi(_countEntry.get_text().c_str());
  if (iter && count > 0) {
    std::unique_lock<std::mutex> lock(_management->Mutex());

    const FixtureType *type = (*iter)[type_columns_.type_];
    FixtureType *project_type = dynamic_cast<FixtureType *>(
        _management->RootFolder().GetChildIfExists(type->Name()));
    if (!project_type) {
      project_type = &_management->GetTheatre().AddFixtureType(*type);
      _management->RootFolder().Add(*project_type);
    }

    for (size_t fixIter = 0; fixIter != static_cast<size_t>(count); ++fixIter) {
      const theatre::Position position =
          _management->GetTheatre().GetFreePosition();
      theatre::Fixture &fixture =
          _management->GetTheatre().AddFixture(*project_type);
      fixture.GetPosition() = position;

      theatre::FixtureControl &control = _management->AddFixtureControl(
          fixture, _management->RootFolder() /* TODO */);
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
        _management->AddSourceValue(control, i);
      }
    }

    lock.unlock();
    _eventHub.EmitUpdate();
    hide();
  }
}

void AddFixtureWindow::onDecCount() {
  int count = std::atoi(_countEntry.get_text().c_str());
  if (count > 1)
    count--;
  else
    count = 1;
  _countEntry.set_text(std::to_string(count));
}

void AddFixtureWindow::onIncCount() {
  int count = std::atoi(_countEntry.get_text().c_str());
  count++;
  _countEntry.set_text(std::to_string(count));
}

}  // namespace glight::gui
