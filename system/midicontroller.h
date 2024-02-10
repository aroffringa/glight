#ifndef SYSTEM_MIDI_CONTROLLER_H_
#define SYSTEM_MIDI_CONTROLLER_H_

#include <algorithm>
#include <atomic>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <alsa/asoundlib.h>

#include "colormap.h"

#include "theatre/color.h"

namespace glight::system {

class MidiController {
 public:
  MidiController(const std::string& device_name);
  ~MidiController() noexcept;

  static std::optional<MidiController> GetController();

  static std::vector<std::string> DeviceNames();

  void SetPixelColor(size_t column, size_t row, const theatre::Color& color,
                     bool blink);

  size_t MatrixWidth() { return 8; }
  size_t MatrixHeight() { return 8; }
  bool HasRowButtons() { return true; }
  bool HasColumnButtons() { return true; }

  constexpr static size_t ButtonQueueSize() { return kButtonQueueSize; }
  unsigned short GetPressedButtons(unsigned char* button_list) {
    const unsigned char n = queued_count_;
    std::copy_n(queued_buttons_, n, button_list);
    const unsigned char pressed = pressed_count_;
    queued_count_ = pressed;
    for (size_t i = 0; i != pressed; ++i) {
      queued_buttons_[i] = pressed_buttons_[i].load();
    }
    return n;
  }
  size_t GetNFaders() const { return 9; }
  unsigned char GetFaderValue(size_t fader_index) {
    return faders_[fader_index];
  }

 private:
  void HandleInput();
  void ProcessInput(unsigned char* data, size_t data_size);
  void ProcessMessage();

  enum class InputState {
    Empty,
    NoteOn,
    NoteOff,
    Controller
  } input_state_ = InputState::Empty;
  unsigned char input_data_[3];
  size_t input_size_ = 0;

  snd_rawmidi_t* in_rmidi_ = nullptr;
  snd_rawmidi_t* out_rmidi_ = nullptr;
  ColorMap color_map_;
  unsigned char current_colors_[64];
  std::thread input_thread_;
  int signal_pipe_fd_[2];
  std::atomic<bool> running_ = false;
  // There are two queues maintained to make sure that a button is never 'lost'.
  // When pressed, the button is added to both queues. When release, it is only
  // removed from the 'pressed' queue. Only when the application reads the
  // buttons, unpressed buttons are also removed from the 'queued' list.
  constexpr static size_t kButtonQueueSize = 16;
  std::atomic<unsigned char> pressed_count_ = 0;
  std::atomic<unsigned char> pressed_buttons_[kButtonQueueSize];
  std::atomic<unsigned char> queued_count_ = 0;
  std::atomic<unsigned char> queued_buttons_[kButtonQueueSize];
  std::atomic<unsigned char> faders_[9];
};

}  // namespace glight::system

#endif
