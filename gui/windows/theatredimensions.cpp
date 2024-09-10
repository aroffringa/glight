#include "theatredimensions.h"

#include "theatre/theatre.h"
#include "theatre/management.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"
#include "gui/units.h"

namespace glight::gui::windows {

TheatreDimensions::TheatreDimensions() {
  const auto enable_set = [&]() { set_button_.set_sensitive(true); };
  grid_.attach(width_label_, 0, 0);
  width_entry_.signal_changed().connect(enable_set);
  grid_.attach(width_entry_, 1, 0);
  grid_.attach(depth_label_, 0, 1);
  depth_entry_.signal_changed().connect(enable_set);
  grid_.attach(depth_entry_, 1, 1);
  grid_.attach(height_label_, 0, 2);
  height_entry_.signal_changed().connect(enable_set);
  grid_.attach(height_entry_, 1, 2);
  grid_.attach(fixture_size_label_, 0, 3);
  fixture_size_entry_.signal_changed().connect(enable_set);
  grid_.attach(fixture_size_entry_, 1, 3);
  set_button_.signal_clicked().connect([&]() { StoreValues(); });
  button_box_.pack_end(set_button_);
  grid_.attach(button_box_, 0, 4, 2, 1);
  add(grid_);
  FillValues();
  set_button_.set_sensitive(false);
  show_all_children();
}

void TheatreDimensions::FillValues() {
  const theatre::Theatre& theatre = Instance::Management().GetTheatre();
  width_entry_.set_text(MetersToString(theatre.Width()));
  depth_entry_.set_text(MetersToString(theatre.Depth()));
  height_entry_.set_text(MetersToString(theatre.Height()));
  fixture_size_entry_.set_text(MetersToString(theatre.FixtureSymbolSize()));
}

void TheatreDimensions::StoreValues() {
  theatre::Theatre& theatre = Instance::Management().GetTheatre();
  theatre.SetWidth(std::atof(width_entry_.get_text().c_str()));
  theatre.SetDepth(std::atof(depth_entry_.get_text().c_str()));
  theatre.SetHeight(std::atof(height_entry_.get_text().c_str()));
  theatre.SetFixtureSymbolSize(
      std::atof(fixture_size_entry_.get_text().c_str()));
  set_button_.set_sensitive(false);
  Instance::Events().EmitUpdate();
}

}  // namespace glight::gui::windows
