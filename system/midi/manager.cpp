#include "manager.h"

#include "controller.h"

namespace glight::system::midi {

Manager::Manager()
    : controller_(Controller::GetController()),
      button_set_(Controller::ButtonQueueSize()),
      selected_colors_(GetNColors()) {
  if (controller_) {
    n_faders_ = controller_->GetNFaders();
    grid_width_ = controller_->MatrixWidth();
    n_pads_ = grid_width_ * controller_->MatrixHeight();
    fixture_colors_.assign(n_pads_, theatre::Color::Black());
    SetTwoColorSelectionMode();
  }
}

Manager::~Manager() = default;

void Manager::SetFixtureColor(size_t column, size_t row,
                              const theatre::Color& color, bool blink) {
  fixture_colors_[column * grid_width_ + row] = color;
  if (mode_ == MidiGridMode::VisualizeFixtures && controller_) {
    controller_->SetPixelColor(column, row, color, false);
  }
}

void Manager::Update() {
  if (controller_) {
    controller_->GetPressedButtons(button_set_);
    controller_->GetPressEvents(button_set_);
    if (button_set_) {
      if (button_set_.Contains(midi::Controller::SceneButton(0))) {
        SetTwoColorSelectionMode();
        mode_ = MidiGridMode::TwoColorSelection;
      }
      if (button_set_.Contains(midi::Controller::SceneButton(1))) {
        for (size_t i = 0; i != 64; ++i) {
          controller_->SetPixelColor(i / 8, i % 8, fixture_colors_[i], false);
        }
        controller_->SetSceneButton(0, ButtonState::Off);
        controller_->SetSceneButton(1, ButtonState::On);
        mode_ = MidiGridMode::VisualizeFixtures;
      }
      if (mode_ == MidiGridMode::TwoColorSelection) {
        for (unsigned char button : button_set_) {
          if (Controller::IsPadButton(button)) {
            const size_t x = Controller::PadButtonX(button);
            const size_t y = Controller::PadButtonY(button);
            const size_t color_index = x < 4 ? 0 : 1;
            const size_t palette_index = (x % 4) * 8 + y;
            if (color_index < 2 && palette_index < 32) {
              // Set the old button back to full on
              const unsigned char old_button =
                  selected_colors_[color_index].first;
              const size_t x_old = Controller::PadButtonX(old_button);
              const size_t y_old = Controller::PadButtonY(old_button);
              const size_t palette_index_old = (x_old % 4) * 8 + y_old;
              const std::vector<theatre::Color> palette =
                  theatre::Color::DefaultSet32();
              controller_->SetPixelColor(x_old, y_old,
                                         palette[palette_index_old], false);
              const theatre::Color color = palette[palette_index];
              controller_->SetPixelColor(x, y, color, true);
              selected_colors_[color_index] = std::make_pair(button, color);
            }
          }
        }
      }
    }
  }
}

void Manager::SetTwoColorSelectionMode() {
  const std::vector<theatre::Color> colors = theatre::Color::DefaultSet32();
  for (size_t i = 0; i != 32; ++i) {
    controller_->SetPixelColor(i / 8, i % 8, colors[i], false);
    controller_->SetPixelColor(i / 8 + 4, i % 8, colors[i], false);
  }
  controller_->SetSceneButton(0, ButtonState::On);
  controller_->SetSceneButton(1, ButtonState::Off);
}

unsigned char Manager::GetFaderValue(size_t fader_index) {
  const unsigned char value = controller_->GetFaderValue(fader_index);
  if (fader_values_.size() <= fader_index)
    fader_values_.resize(fader_index + 1);
  if (value != fader_values_[fader_index]) {
    fader_values_[fader_index] = value;
    controller_->SetTrackButton(fader_index,
                                value ? ButtonState::On : ButtonState::Off);
  }
  return value;
}

}  // namespace glight::system::midi
