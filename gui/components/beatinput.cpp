#include "beatinput.h"

#include <sstream>

namespace glight::gui {
namespace {
inline constexpr size_t NVALUES = 8;
inline const double values[NVALUES] = {0.25, 0.5, 1.0, 2.0, 3.0, 4.0, 6.0, 8.0};
}  // namespace
inline const std::string kStringValues[]{"1/4", "1/2", "1", "2",
                                         "3",   "4",   "6", "8"};

BeatInput::BeatInput(double value) : scale_(0, NVALUES, 1) {
  Initialize(value);
}

BeatInput::BeatInput(const std::string &label, double value)
    : caption_label_(label), scale_(0, NVALUES, 1) {
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
  pack_end(value_label_, false, false);

  show_all_children();
}

double BeatInput::ValueToScale(double value) {
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

void BeatInput::OnScaleChanged() {
  if (recursion_lock_.IsFirst()) {
    RecursionLock::Token token(recursion_lock_);
    const size_t index = scale_.get_value();
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