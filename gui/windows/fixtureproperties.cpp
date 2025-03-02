#include "fixtureproperties.h"

#include "gui/eventtransmitter.h"
#include "gui/fixtureselection.h"
#include "gui/instance.h"
#include "gui/units.h"

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "theatre/fixture.h"
#include "theatre/fixturecontrol.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/theatre.h"

#include <algorithm>

namespace glight::gui::windows {

FixtureProperties::FixtureProperties() {
  set_title("Glight - fixture types");
  set_size_request(200, 400);

  auto update_lambda = [&]() { FixtureProperties::update(); };
  update_controllables_connection_ =
      Instance::Events().SignalUpdateControllables().connect(update_lambda);
  selection_change_connection_ =
      Instance::Selection().SignalChange().connect(update_lambda);

  main_grid_.attach(height_label_, 0, 0);
  main_grid_.attach(height_entry_, 1, 0);
  main_grid_.attach(direction_label_, 0, 1);
  main_grid_.attach(direction_entry_, 1, 1);
  main_grid_.attach(static_tilt_label_, 0, 2);
  main_grid_.attach(static_tilt_entry_, 1, 2);
  main_grid_.attach(upside_down_cb_, 0, 3, 2, 1);
  main_grid_.attach(phase_label_, 0, 4);
  main_grid_.attach(phase_entry_, 1, 4);

  button_box_.set_homogeneous(true);

  set_button_.signal_clicked().connect([&]() { onSetClicked(); });
  button_box_.pack_start(set_button_);
  main_grid_.attach(button_box_, 0, 5, 2, 1);

  add(main_grid_);
  main_grid_.show_all();

  update();
}

void FixtureProperties::update() {
  const std::vector<system::ObservingPtr<theatre::Fixture>> &fixtures =
      Instance::Selection().Selection();
  if (fixtures.empty()) {
    set_sensitive(false);
  } else {
    set_sensitive(true);
    const theatre::Fixture &first_fixture = *fixtures.front();
    height_entry_.set_text(MetersToString(first_fixture.GetPosition().Z()));
    direction_entry_.set_text(AngleToNiceString(first_fixture.Direction()));
    static_tilt_entry_.set_text(AngleToNiceString(first_fixture.StaticTilt()));
    upside_down_cb_.set_active(first_fixture.IsUpsideDown());
    phase_entry_.set_text(std::to_string(first_fixture.ElectricPhase()));
  }
}

void FixtureProperties::onSetClicked() {
  const double height = std::atof(height_entry_.get_text().c_str());
  const double direction_degrees =
      std::atof(direction_entry_.get_text().c_str());
  const double direction =
      std::clamp(direction_degrees, 0.0, 360.0) * M_PI / 180.0;
  const double tilt_degrees = std::atof(static_tilt_entry_.get_text().c_str());
  const double tilt = std::clamp(tilt_degrees, -180.0, 180.0) * M_PI / 180.0;
  const bool upside_down = upside_down_cb_.get_active();
  std::unique_lock lock(Instance::Management().Mutex());
  const std::vector<system::ObservingPtr<theatre::Fixture>> &fixtures =
      Instance::Selection().Selection();
  for (const system::ObservingPtr<theatre::Fixture> &fixture : fixtures) {
    fixture->GetPosition().Z() = height;
    fixture->SetDirection(direction);
    fixture->SetStaticTilt(tilt);
    fixture->SetUpsideDown(upside_down);
    fixture->SetElectricPhase(std::atoi(phase_entry_.get_text().c_str()));
  }
  lock.unlock();
  Instance::Events().EmitUpdate();
}

}  // namespace glight::gui::windows
