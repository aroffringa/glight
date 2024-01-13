#include "beatinput.h"

#include <sstream>

namespace glight::gui {
namespace {
inline constexpr size_t kNValues = 10;
inline const double values[kNValues] = {0.25, 0.5, 1.0, 2.0, 3.0,
                                        4.0,  6.0, 8.0, 16.0};
}  // namespace
inline const std::string kStringValues[kNValues]{
    "1/4", "1/2", " 1 ", " 2 ", " 3 ", " 4 ", " 6 ", " 8 ", " 16", "32"};

BeatInput::BeatInput(double value)
    : scale_(Gtk::Adjustment::create(
                 0, 0, static_cast<double>(kNValues - 1) + 0.1, 1),
             Gtk::ORIENTATION_HORIZONTAL) {
  Initialize(value);
}

BeatInput::BeatInput(const std::string &label, double value)
    : caption_label_(label),
      scale_(Gtk::Adjustment::create(
                 0, 0, static_cast<double>(kNValues - 1) + 0.1, 1),
             Gtk::ORIENTATION_HORIZONTAL) {
  caption_label_.set_halign(Gtk::ALIGN_END);
  pack_start(caption_label_, false, false);

  Initialize(value);
}

void BeatInput::Initialize(double value) {
  scale_.set_size_request(150, 0);
  const size_t index = ValueToScale(value);
  scale_.set_value(index);
  scale_.set_round_digits(0);
  scale_.set_draw_value(false);
  scale_.signal_value_changed().connect([&]() { OnScaleChanged(); });
  pack_start(scale_, true, true);

  SetValueLabel(index);
  value_label_.set_width_chars(3);
  pack_end(value_label_, false, false);

  show_all_children();
}

double BeatInput::ValueToScale(double value) {
  for (size_t i = 0; i != kNValues - 1; ++i) {
    if (values[i + 1] > value) {
      if (value - values[i] < values[i + 1] - value)
        return i;
      else
        return i + 1;
    }
  }
  return kNValues - 1;
}

void BeatInput::OnScaleChanged() {
  if (recursion_lock_.IsFirst()) {
    RecursionLock::Token token(recursion_lock_);
    const size_t index = std::min<size_t>(kNValues - 1, scale_.get_value());
    SetValueLabel(index);
    const double new_value = values[index];
    signal_value_changed_.emit(new_value);
  }
}

void BeatInput::SetValue(double new_value) {
  RecursionLock::Token token(recursion_lock_);
  const size_t index = ValueToScale(new_value);
  scale_.set_value(index);
  SetValueLabel(index);
  signal_value_changed_.emit(new_value);
}

void BeatInput::SetValueLabel(unsigned index) {
  value_label_.set_text(kStringValues[index]);
}

}  // namespace glight::gui
