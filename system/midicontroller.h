#ifndef SYSTEM_MIDI_CONTROLLER_H_
#define SYSTEM_MIDI_CONTROLLER_H_

#include <string>
#include <vector>

#include <alsa/asoundlib.h>

#include "colormap.h"

#include "theatre/color.h"

namespace glight::system {

class MidiController {
 public:
  MidiController();
  ~MidiController() noexcept;

  static std::vector<std::string> DeviceNames();

  void SetPixelColor(size_t column, size_t row, const theatre::Color& color,
                     bool blink);

  size_t MatrixWidth() { return 8; }
  size_t MatrixHeight() { return 8; }
  bool HasRowButtons() { return true; }
  bool HasColumnButtons() { return true; }

 private:
  snd_rawmidi_t* in_rmidi_ = nullptr;
  snd_rawmidi_t* out_rmidi_ = nullptr;
  ColorMap color_map_;
  unsigned char current_colors_[64];
};

}  // namespace glight::system

#endif
