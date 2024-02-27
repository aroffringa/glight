#include "fixtureproperties.h"

#include "gui/eventtransmitter.h"
#include "gui/fixtureselection.h"
#include "gui/instance.h"

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "theatre/fixture.h"
#include "theatre/fixturecontrol.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/theatre.h"

#include <algorithm>
#include <format>

namespace glight::gui::windows {

FixtureProperties::FixtureProperties() {
  set_title("Glight - fixture types");
  set_size_request(200, 400);

  auto update_lambda = [&]() { FixtureProperties::update(); };
  update_controllables_connection_ =
      Instance::Events().SignalUpdateControllables().connect(update_lambda);
  selection_change_connection_ =
      Instance::Selection().SignalChange().connect(update_lambda);

  main_grid_.attach(direction_label_, 0, 0);
  main_grid_.attach(direction_entry_, 1, 0);
  main_grid_.attach(tilt_label_, 0, 1);
  main_grid_.attach(tilt_entry_, 1, 1);
  main_grid_.attach(upside_down_cb_, 0, 2, 2, 1);

  button_box_.set_homogeneous(true);

  set_button_.signal_clicked().connect([&]() { onSetClicked(); });
  button_box_.pack_start(set_button_);
  main_grid_.attach(button_box_, 0, 3, 2, 1);

  add(main_grid_);
  main_grid_.show_all();

  update();
}

void FixtureProperties::update() {
  const std::vector<theatre::Fixture *> &fixtures =
      Instance::Selection().Selection();
  if (fixtures.empty()) {
    set_sensitive(false);
  } else {
    set_sensitive(true);
    const theatre::Fixture &first_fixture = *fixtures.front();
    direction_entry_.set_text(
        std::format("{:.1f}", first_fixture.Direction() * 180.0 / M_PI));
    tilt_entry_.set_text(
        std::format("{:.1f}", first_fixture.Tilt() * 180.0 / M_PI));
    upside_down_cb_.set_active(first_fixture.IsUpsideDown());
  }
}

void FixtureProperties::onSetClicked() {
  const double direction_degrees =
      std::atof(direction_entry_.get_text().c_str());
  const double direction =
      std::clamp(direction_degrees, 0.0, 360.0) * M_PI / 180.0;
  const double tilt_degrees = std::atof(tilt_entry_.get_text().c_str());
  const double tilt = std::clamp(tilt_degrees, -180.0, 180.0) * M_PI / 180.0;
  const bool upside_down = upside_down_cb_.get_active();
  const std::vector<theatre::Fixture *> &fixtures =
      Instance::Selection().Selection();
  for (theatre::Fixture *fixture : fixtures) {
    fixture->SetDirection(direction);
    fixture->SetTilt(tilt);
    fixture->SetUpsideDown(upside_down);
  }
}

}  // namespace glight::gui::windows
