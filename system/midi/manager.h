#ifndef GLIGHT_SYSTEM_MIDI_MANAGER_H_
#define GLIGHT_SYSTEM_MIDI_MANAGER_H_

#include <optional>
#include <vector>

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
  theatre::Color GetColor(size_t index) const {
    return selected_colors_[index].second;
  }

  void SetFixtureColor(size_t column, size_t row, const theatre::Color& color,
                       bool blink);
  void Update();

 private:
  void SetTwoColorSelectionMode();

  std::unique_ptr<Controller> controller_;
  MidiGridMode mode_ = MidiGridMode::TwoColorSelection;
  size_t n_faders_ = 0;
  size_t n_pads_ = 0;
  size_t grid_width_ = 0;
  std::vector<unsigned char> fader_values_;
  std::vector<theatre::Color> fixture_colors_;
  ButtonSet button_set_;
  std::vector<std::pair<size_t, theatre::Color>> selected_colors_;
};

}  // namespace glight::system::midi

#endif
