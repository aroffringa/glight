#include "powermonitor.h"

#include <iomanip>
#include <map>
#include <sstream>

#include <glibmm/main.h>

#include "theatre/management.h"
#include "theatre/theatre.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

namespace glight::gui::components {

PowerMonitor::PowerMonitor() {
  set_column_spacing(8);
  set_margin_bottom(6);
}

void PowerMonitor::Start() {
  timeout_connection_ = Glib::signal_timeout().connect(
      [&]() {
        TimeUpdate();
        return true;
      },
      250);
  update_connection_ = Instance::Events().SignalUpdateControllables().connect(
      [&]() { Update(); });
  Update();
}

void PowerMonitor::Update() {
  const theatre::Management& management = Instance::Management();
  snapshot_ = management.PrimarySnapshot();
  UpdateValues();
}

void PowerMonitor::TimeUpdate() {
  const theatre::Management& management = Instance::Management();
  glight::theatre::ValueSnapshot primary_snapshot =
      management.PrimarySnapshot();
  if (snapshot_ != primary_snapshot) {
    snapshot_ = std::move(primary_snapshot);
    UpdateValues();
  }
}

void PowerMonitor::SetRow(size_t row_index, double used_power,
                          double max_power) {
  const double fraction = max_power == 0.0 ? 0.0 : used_power / max_power;
  rows_[row_index].progress_bar_.set_fraction(fraction);
  std::ostringstream text;
  if (row_index == 0)
    text << "Total power: ";
  else
    text << "Phase " << row_index << " :  ";
  text << std::fixed << std::setprecision(1) << used_power * 1e-3 << " KW / "
       << std::fixed << std::setprecision(1) << max_power * 1e-3 << " KW";
  rows_[row_index].label_.set_text(text.str());
}

std::map<size_t, std::pair<double, double>> GetPowerPerPhase(
    const theatre::ValueSnapshot& snapshot) {
  const theatre::Theatre& theatre = Instance::Management().GetTheatre();
  std::map<size_t, std::pair<double, double>> phases;
  for (const system::TrackablePtr<theatre::Fixture>& fixture :
       theatre.Fixtures()) {
    const double fixture_power = fixture->Type().GetPower(*fixture, snapshot);
    std::pair<double, double>& phase_power = phases[fixture->ElectricPhase()];
    phase_power.first += fixture_power;
    phase_power.second += fixture->Type().MaxPower();
  }
  if (phases.empty()) phases.emplace(0, std::make_pair(0.0, 0.0));
  return phases;
}

void PowerMonitor::UpdateValues() {
  const std::map<size_t, std::pair<double, double>> phases =
      GetPowerPerPhase(snapshot_);
  const size_t n_rows = phases.size() > 1 ? phases.size() + 1 : 1;
  while (rows_.size() < n_rows) {
    Row& row = rows_.emplace_back();
    const size_t y = rows_.size() - 1;
    attach(row.label_, 0, y);
    row.label_.show();
    attach(row.progress_bar_, 1, y);
    row.progress_bar_.set_hexpand(true);
    row.progress_bar_.show();
  }
  while (rows_.size() > n_rows) {
    remove_row(rows_.size() - 1);
    rows_.pop_back();
  }

  double total_usage = 0.0;
  double total_maximum = 0.0;
  for (const auto& phase : phases) {
    total_usage += phase.second.first;
    total_maximum += phase.second.second;
  }

  auto phase_iter = phases.begin();
  for (size_t phases_index = 0; phases_index != n_rows - 1; ++phases_index) {
    SetRow(phases_index + 1, phase_iter->second.first,
           phase_iter->second.second);
    ++phase_iter;
  }
  SetRow(0, total_usage, total_maximum);
}

}  // namespace glight::gui::components
