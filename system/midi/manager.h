#ifndef GLIGHT_SYSTEM_MIDI_MANAGER_H_
#define GLIGHT_SYSTEM_MIDI_MANAGER_H_

#include <optional>
#include <vector>

#include "system/optionalnumber.h"
#include "system/midi/buttonset.h"

#include "theatre/color.h"

namespace glight::system::midi {

class Controller;

enum class MidiGridMode { TwoColorSelection, VisualizeFixtures };

class Manager {
 public:
  Manager();
  ~Manager();

  size_t GetNPads() const { return n_pads_; }

  size_t GetNFaders() const { return n_faders_; }
  unsigned char GetFaderValue(size_t fader_index);

  size_t GetNColors() const { return 2; }
  std::optional<theatre::Color> GetColor(size_t index) {
    if (std::get<1>(selected_colors_[index])) {
      std::get<1>(selected_colors_[index]) = false;
      return std::get<2>(selected_colors_[index]);
    } else {
      return {};
    }
  }

  void SetFixtureColor(size_t column, size_t row, const theatre::Color& color,
                       bool blink);
  void Update();

  void SetBeat(system::OptionalNumber<double> beat);

 private:
  void SetTwoColorSelectionMode();
  void HandleButtonPress(const ButtonSet& button_set);
  void HandleButtonRelease(const ButtonSet& button_set);

  std::unique_ptr<Controller> controller_;
  MidiGridMode mode_ = MidiGridMode::TwoColorSelection;
  size_t n_faders_ = 0;
  size_t n_pads_ = 0;
  size_t grid_width_ = 0;
  std::vector<std::pair<unsigned char, bool>> fader_values_;
  std::vector<theatre::Color> fixture_colors_;
  ButtonSet button_set_;
  // 1) Index of button associated with selected color,
  // 2) true if the value changed since the last update, 3) color.
  std::vector<std::tuple<size_t, bool, theatre::Color>> selected_colors_;
  system::OptionalNumber<double> beat_;
};

}  // namespace glight::system::midi

#endif
