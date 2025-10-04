#include "fixturetypeswindow.h"

#include <algorithm>

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "gui/eventtransmitter.h"
#include "gui/fixtureselection.h"
#include "gui/instance.h"
#include "gui/units.h"

#include "theatre/fixture.h"
#include "theatre/fixturecontrol.h"
#include "theatre/fixturetype.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/theatre.h"

namespace glight::gui::windows {

using system::ObservingPtr;
using system::TrackablePtr;
using theatre::FixtureMode;
using theatre::FixtureType;

FixtureTypesWindow::FixtureTypesWindow() : functions_frame_(*this) {
  set_title("Glight - fixture types");
  set_size_request(200, 400);

  update_controllables_connection_ =
      Instance::Events().SignalUpdateControllables().connect(
          [&]() { FixtureTypesWindow::update(); });

  // Left part
  tree_model_ = Gtk::TreeStore::create(list_columns_);

  tree_view_.set_model(tree_model_);
  tree_view_.append_column("Name", list_columns_.name_);
  tree_view_.append_column("Used", list_columns_.in_use_);
  tree_view_.append_column("Functions", list_columns_.functions_);
  tree_view_.get_selection()->signal_changed().connect(
      [&]() { onSelectionChanged(); });
  fillList();
  type_scrollbars_.add(tree_view_);

  type_scrollbars_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  left_box_.pack_start(type_scrollbars_);

  paned_.add1(left_box_);
  left_box_.set_hexpand(true);
  left_box_.set_vexpand(true);

  // Right part
  right_grid_.attach(name_label_, 0, 0);
  right_grid_.attach(name_entry_, 1, 0, 2, 1);
  name_entry_.set_hexpand(true);

  right_grid_.attach(short_name_label_, 0, 1);
  right_grid_.attach(short_name_entry_, 1, 1, 2, 1);
  short_name_entry_.set_hexpand(true);

  right_grid_.attach(class_label_, 0, 2);
  const std::vector<theatre::FixtureClass> classes =
      theatre::GetFixtureClassList();
  for (theatre::FixtureClass c : classes)
    class_combo_.append(std::string(ToString(c)));
  right_grid_.attach(class_combo_, 1, 2, 2, 1);
  class_combo_.set_hexpand(true);

  right_grid_.attach(beam_angle_label_, 0, 3);
  right_grid_.attach(min_beam_angle_entry_, 1, 3);
  right_grid_.attach(max_beam_angle_entry_, 2, 3);

  right_grid_.attach(pan_label_, 0, 4);
  right_grid_.attach(min_pan_entry_, 1, 4);
  right_grid_.attach(max_pan_entry_, 2, 4);

  right_grid_.attach(tilt_label_, 0, 5);
  right_grid_.attach(min_beam_tilt_entry_, 1, 5);
  right_grid_.attach(max_beam_tilt_entry_, 2, 5);

  right_grid_.attach(brightness_label_, 0, 6);
  right_grid_.attach(brightness_entry_, 1, 6, 2, 1);

  right_grid_.attach(max_power_label_, 0, 7);
  right_grid_.attach(max_power_entry_, 1, 7, 2, 1);

  right_grid_.attach(idle_power_label_, 0, 8);
  right_grid_.attach(idle_power_entry_, 1, 8, 2, 1);

  right_grid_.attach(functions_frame_, 0, 10, 3, 1);
  functions_frame_.set_vexpand(true);
  functions_frame_.set_hexpand(true);

  paned_.add(right_grid_);
  right_grid_.set_hexpand(true);
  right_grid_.set_vexpand(true);
  right_grid_.set_row_spacing(5);
  right_grid_.set_column_spacing(5);

  main_grid_.attach(paned_, 0, 0, 1, 1);

  // Buttons at bottom
  button_box_.set_homogeneous(true);

  new_button_.set_image_from_icon_name("document-new");
  new_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onNewButtonClicked), false);
  button_box_.pack_start(new_button_);

  remove_button_.set_image_from_icon_name("edit-delete");
  remove_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onRemoveClicked));
  button_box_.pack_start(remove_button_);

  save_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onSaveClicked));
  button_box_.pack_start(save_button_);

  main_grid_.attach(button_box_, 0, 1, 1, 1);
  button_box_.set_hexpand(true);
  button_box_.set_vexpand(false);

  add(main_grid_);
  main_grid_.show_all();

  onSelectionChanged();
}

void FixtureTypesWindow::fillList() {
  RecursionLock::Token token(recursion_lock_);
  const auto [selected_type, selected_mode] = GetSelected();
  tree_model_->clear();

  theatre::Management &management = Instance::Management();
  std::lock_guard<std::mutex> lock(management.Mutex());
  const std::vector<TrackablePtr<FixtureType>> &project_types =
      management.GetTheatre().FixtureTypes();
  for (const TrackablePtr<FixtureType> &project_type : project_types) {
    Gtk::TreeModel::iterator iter = tree_model_->append();
    const Gtk::TreeModel::Row &row = *iter;
    row[list_columns_.fixture_type_] = project_type.Get();
    row[list_columns_.name_] = project_type->Name();
    row[list_columns_.in_use_] = management.GetTheatre().IsUsed(*project_type);
    if (selected_type && selected_type == project_type.Get()) {
      tree_view_.get_selection()->select(row);
    }
    for (FixtureMode &mode : project_type->Modes()) {
      Gtk::TreeModel::iterator child_iter = tree_model_->append(row.children());
      const Gtk::TreeModel::Row &child_row = *child_iter;
      child_row[list_columns_.fixture_mode_] = &mode;
      child_row[list_columns_.name_] = mode.Name();
      child_row[list_columns_.functions_] = FunctionSummary(mode);
      child_row[list_columns_.in_use_] = management.GetTheatre().IsUsed(mode);
    }
  }
}

void FixtureTypesWindow::onNewButtonClicked() {
  if (!tree_model_->children().empty()) {
    Gtk::TreeNodeChildren::iterator last = tree_model_->children().end();
    --last;
    // If the last row is already a new, unsaved type, ignore the request
    if ((*last)[list_columns_.fixture_type_] == nullptr) return;
  }
  Gtk::TreeModel::iterator iter = tree_model_->append();
  const Gtk::TreeModel::Row &row = *iter;
  row[list_columns_.fixture_type_] = nullptr;
  row[list_columns_.name_] = {};
  row[list_columns_.functions_] = {};
  row[list_columns_.in_use_] = false;
  tree_view_.get_selection()->select(iter);
}

void FixtureTypesWindow::onRemoveClicked() {
  const auto [type, mode] = GetSelected();
  if (type) {
    {
      theatre::Management &management = Instance::Management();
      std::lock_guard<std::mutex> lock(management.Mutex());
      management.RemoveFixtureType(*type);
      Instance::Selection().UpdateAfterDelete();
    }
    Instance::Events().EmitUpdate();
  } else {
    const Gtk::TreeModel::const_iterator selected =
        tree_view_.get_selection()->get_selected();
    if (selected) tree_model_->erase(selected);
  }
}

void FixtureTypesWindow::onSaveClicked() {
  auto [type, mode] = GetSelected();
  if (mode) {
    const bool is_used = Instance::Management().GetTheatre().IsUsed(*type);
    if (!is_used) {
      mode->SetFunctions(functions_frame_.GetFunctions());
    }
  } else {
    if (!type) {
      ObservingPtr<FixtureType> new_type =
          Instance::Management()
              .GetTheatre()
              .AddFixtureType(system::MakeTrackable<FixtureType>())
              .GetObserver<FixtureType>();
      type = new_type.Get();
      Instance::Management().RootFolder().Add(std::move(new_type));
    }
    type->SetName(name_entry_.get_text());
    type->SetShortName(short_name_entry_.get_text());

    const double min_beam_angle =
        std::atof(min_beam_angle_entry_.get_text().c_str());
    type->SetMinBeamAngle(std::clamp(min_beam_angle, 0.0, 360.0) * M_PI /
                          180.0);
    const double max_beam_angle =
        std::atof(max_beam_angle_entry_.get_text().c_str());
    type->SetMaxBeamAngle(std::clamp(max_beam_angle, 0.0, 360.0) * M_PI /
                          180.0);

    const double min_pan = std::atof(min_pan_entry_.get_text().c_str());
    type->SetMinPan(std::clamp(min_pan, -3600.0, 3600.0) * M_PI / 180.0);
    const double max_pan = std::atof(max_pan_entry_.get_text().c_str());
    type->SetMaxPan(std::clamp(max_pan, -3600.0, 3600.0) * M_PI / 180.0);

    const double min_tilt = std::atof(min_beam_tilt_entry_.get_text().c_str());
    type->SetMinTilt(std::clamp(min_tilt, -3600.0, 3600.0) * M_PI / 180.0);
    const double max_tilt = std::atof(max_beam_tilt_entry_.get_text().c_str());
    type->SetMaxTilt(std::clamp(max_tilt, -3600.0, 3600.0) * M_PI / 180.0);

    const double brightness = std::atof(brightness_entry_.get_text().c_str());
    type->SetBrightness(std::clamp(brightness, 0.0, 100.0));
    const unsigned max_power =
        std::max(0LL, std::atoll(max_power_entry_.get_text().c_str()));
    type->SetMaxPower(max_power);
    const unsigned idle_power =
        std::max(0LL, std::atoll(idle_power_entry_.get_text().c_str()));
    type->SetIdlePower(idle_power);
    type->SetFixtureClass(
        theatre::GetFixtureClass(class_combo_.get_active_text().data()));
    Instance::Events().EmitUpdate();
    Select(*type);
  }
}

void FixtureTypesWindow::Select(const FixtureMode &selection) {
  Gtk::TreeModel::Children children = tree_model_->children();
  for (Gtk::TreeRow row : children) {
    if (row[list_columns_.fixture_mode_] == &selection) {
      tree_view_.get_selection()->select(row);
      break;
    }
  }
}

void FixtureTypesWindow::Select(const FixtureType &selection) {
  Gtk::TreeModel::Children children = tree_model_->children();
  for (Gtk::TreeRow row : children) {
    if (row[list_columns_.fixture_type_] == &selection) {
      tree_view_.get_selection()->select(row);
      break;
    }
  }
}

std::pair<FixtureType *, FixtureMode *> FixtureTypesWindow::GetSelected() {
  Glib::RefPtr<Gtk::TreeSelection> selection = tree_view_.get_selection();
  const Gtk::TreeModel::const_iterator selected = selection->get_selected();
  if (selected) {
    FixtureMode *mode = (*selected)[list_columns_.fixture_mode_];
    FixtureType *type = (*selected)[list_columns_.fixture_type_];
    if (mode)
      return {nullptr, mode};
    else
      return {type, nullptr};
  } else {
    return {nullptr, nullptr};
  }
}

void FixtureTypesWindow::SelectFixtures(const FixtureMode &mode) {
  const std::vector<system::TrackablePtr<theatre::Fixture>> &fixtures =
      Instance::Management().GetTheatre().Fixtures();
  std::vector<system::ObservingPtr<theatre::Fixture>> selected_fixtures;
  for (const system::TrackablePtr<theatre::Fixture> &fixture : fixtures) {
    if (&fixture->Mode() == &mode)
      selected_fixtures.emplace_back(fixture.GetObserver());
  }
  Instance::Selection().SetSelection(std::move(selected_fixtures));
}

void FixtureTypesWindow::SelectFixtures(const FixtureType &type) {
  const std::vector<system::TrackablePtr<theatre::Fixture>> &fixtures =
      Instance::Management().GetTheatre().Fixtures();
  std::vector<system::ObservingPtr<theatre::Fixture>> selected_fixtures;
  for (const system::TrackablePtr<theatre::Fixture> &fixture : fixtures) {
    if (&fixture->Mode().Type() == &type)
      selected_fixtures.emplace_back(fixture.GetObserver());
  }
  Instance::Selection().SetSelection(std::move(selected_fixtures));
}

void FixtureTypesWindow::onSelectionChanged() {
  if (recursion_lock_.IsFirst()) {
    RecursionLock::Token token(recursion_lock_);
    Glib::RefPtr<Gtk::TreeSelection> selection = tree_view_.get_selection();
    const auto [type, mode] = GetSelected();
    const bool has_selection = type || mode;
    remove_button_.set_sensitive(has_selection && !layout_locked_);
    save_button_.set_sensitive(has_selection && !layout_locked_);
    right_grid_.set_sensitive(has_selection && !layout_locked_);
    if (mode) {
      ShowTypeWidgets(false);
      const bool is_used = Instance::Management().GetTheatre().IsUsed(*mode);
      functions_frame_.set_sensitive(!is_used && !layout_locked_);
      functions_frame_.SetFunctions(mode->Functions());
    } else if (type) {
      ShowTypeWidgets(true);
      SelectFixtures(*type);
      const bool is_used = Instance::Management().GetTheatre().IsUsed(*type);
      name_entry_.set_text(type->Name());
      short_name_entry_.set_text(type->ShortName());

      min_beam_angle_entry_.set_text(AngleToNiceString(type->MinBeamAngle()));
      max_beam_angle_entry_.set_text(AngleToNiceString(type->MaxBeamAngle()));

      min_pan_entry_.set_text(AngleToNiceString(type->MinPan()));
      max_pan_entry_.set_text(AngleToNiceString(type->MaxPan()));

      min_beam_tilt_entry_.set_text(AngleToNiceString(type->MinTilt()));
      max_beam_tilt_entry_.set_text(AngleToNiceString(type->MaxTilt()));

      brightness_entry_.set_text(std::to_string(type->Brightness()));
      class_combo_.set_sensitive(!is_used && !layout_locked_);
      class_combo_.set_active_text(
          std::string(ToString(type->GetFixtureClass())));

      max_power_entry_.set_text(std::to_string(type->MaxPower()));
      idle_power_entry_.set_text(std::to_string(type->IdlePower()));
    } else {
      ShowTypeWidgets(true);
      name_entry_.set_text("");
      short_name_entry_.set_text("");
      min_beam_angle_entry_.set_text("30");
      max_beam_angle_entry_.set_text("30");
      min_pan_entry_.set_text("0");
      max_pan_entry_.set_text("0");
      min_beam_tilt_entry_.set_text("0");
      max_beam_tilt_entry_.set_text("0");
      brightness_entry_.set_text("10");
      class_combo_.set_sensitive(!layout_locked_);
      class_combo_.set_active_text(
          std::string(ToString(theatre::FixtureClass::Par)));
      max_power_entry_.set_text("0");
      idle_power_entry_.set_text("0");
      functions_frame_.SetFunctions({});
    }
  }
}

void FixtureTypesWindow::ShowTypeWidgets(bool visible) {
  const bool not_visible = !visible;

  name_label_.set_visible(visible);
  name_entry_.set_visible(visible);
  short_name_label_.set_visible(visible);
  short_name_entry_.set_visible(visible);
  class_label_.set_visible(visible);
  class_combo_.set_visible(visible);

  beam_angle_label_.set_visible(visible);
  min_beam_angle_entry_.set_visible(visible);
  max_beam_angle_entry_.set_visible(visible);

  pan_label_.set_visible(visible);
  min_pan_entry_.set_visible(visible);
  max_pan_entry_.set_visible(visible);

  tilt_label_.set_visible(visible);
  min_beam_tilt_entry_.set_visible(visible);
  max_beam_tilt_entry_.set_visible(visible);

  brightness_label_.set_visible(visible);
  brightness_entry_.set_visible(visible);

  max_power_label_.set_visible(visible);
  max_power_entry_.set_visible(visible);

  idle_power_label_.set_visible(visible);
  idle_power_entry_.set_visible(visible);

  functions_frame_.set_visible(not_visible);
}

}  // namespace glight::gui::windows
