#include "durationinput.h"

#include <sstream>

namespace glight::gui {
namespace {
inline constexpr size_t NVALUES = 21;
inline const double values[NVALUES] = {
    0.0,    40.0,    80.0,    160.0,   300.0,   500.0,    750.0,
    1000.0, 1500.0,  2000.0,  2500.0,  3000.0,  4000.0,   5000.0,
    7500.0, 10000.0, 15000.0, 30000.0, 60000.0, 120000.0, 300000.0};
}  // namespace

DurationInput::DurationInput(double value) : scale_(0, NVALUES, 1) {
  Initialize(value);
}

DurationInput::DurationInput(const std::string &label, double value)
    : label_(label), scale_(0, NVALUES, 1) {
  label_.set_halign(Gtk::ALIGN_END);
  pack_start(label_, false, false);

  Initialize(value);
}

void DurationInput::Initialize(double value) {
  scale_.set_size_request(150, 0);
  scale_.set_value(ValueToScale(value));
  scale_.set_round_digits(0);
  scale_.set_draw_value(false);
  scale_.signal_value_changed().connect([&]() { onScaleChanged(); });
  pack_start(scale_, true, true);

  SetEntry(value);
  entry_.set_max_length(6);
  entry_.set_width_chars(6);
  entry_.signal_changed().connect([&]() { OnEntryChanged(); });
  pack_end(entry_, false, false);

  show_all_children();
}

double DurationInput::ValueToScale(double value) {
  for (size_t i = 0; i != NVALUES - 1; ++i) {
    if (values[i + 1] > value) {
      if (value - values[i] < values[i + 1] - value)
        return i;
      else
        return i + 1;
    }
  }
  return NVALUES - 1;
}

void DurationInput::onScaleChanged() {
  if (recursion_lock_.IsFirst()) {
    RecursionLock::Token token(recursion_lock_);
    size_t index = scale_.get_value();
    double newValue = values[index];
    SetEntry(newValue);
    signal_value_changed_.emit(newValue);
  }
}

void DurationInput::OnEntryChanged() {
  if (recursion_lock_.IsFirst()) {
    RecursionLock::Token token(recursion_lock_);
    double newValue = atof(entry_.get_text().c_str()) * 1e3;
    scale_.set_value(ValueToScale(newValue));
    signal_value_changed_.emit(newValue);
  }
}

void DurationInput::SetValue(double newValue) {
  RecursionLock::Token token(recursion_lock_);
  scale_.set_value(ValueToScale(newValue));
  SetEntry(newValue);
  signal_value_changed_.emit(newValue);
}

void DurationInput::SetEntry(double newValue) {
  std::ostringstream str;
  str << (round(newValue * 0.1) * 1e-2);
  entry_.set_text(str.str());
}

}  // namespace glight::gui
