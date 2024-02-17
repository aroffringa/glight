#ifndef SYSTEM_MIDI_CONTROLLER_H_
#define SYSTEM_MIDI_CONTROLLER_H_

#include <algorithm>
#include <atomic>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <alsa/asoundlib.h>

#include "buttonset.h"

#include "system/colormap.h"

#include "theatre/color.h"

namespace glight::system::midi {

enum class ButtonState { Off, On, Blink };

class Controller {
 public:
  Controller(const std::string& device_name);
  ~Controller() noexcept;

  static std::unique_ptr<Controller> GetController();

  static std::vector<std::string> DeviceNames();

  void SetPixelColor(size_t column, size_t row, const theatre::Color& color,
                     bool blink);

  void SetTrackButton(size_t index, ButtonState state);
  void SetSceneButton(size_t index, ButtonState state);

  size_t MatrixWidth() { return 8; }
  size_t MatrixHeight() { return 8; }
  bool HasRowButtons() { return true; }
  bool HasColumnButtons() { return true; }

  constexpr static size_t ButtonQueueSize() { return kButtonQueueSize; }

  void GetPressEvents(ButtonSet& button_set) {
    button_set.SetSize(press_event_count_);
    std::copy_n(press_event_buttons_, button_set.Size(), button_set.Data());
    press_event_count_ = 0;
  }

  /**
   * Get the currently pressed buttons. If a button was pressed after
   * the last call, but release before this call, it will still be
   * returned as pressed in order not to lose a button press.
   */
  void GetPressedButtons(ButtonSet& button_set) {
    button_set.SetSize(queued_count_);
    std::copy_n(queued_buttons_, button_set.Size(), button_set.Data());
    const unsigned char pressed = pressed_count_;
    queued_count_ = pressed;
    for (size_t i = 0; i != pressed; ++i) {
      queued_buttons_[i] = pressed_buttons_[i].load();
    }
  }

  size_t GetNFaders() const { return 9; }
  unsigned char GetFaderValue(size_t fader_index) {
    return faders_[fader_index];
  }

  static bool IsPadButton(unsigned char button_index) {
    return button_index < 64;
  }
  static size_t PadButtonX(unsigned char button_index) {
    return button_index % 8;
  }
  static size_t PadButtonY(unsigned char button_index) {
    return button_index / 8;
  }

  static char SceneButton(size_t index) { return 0x70 + index; }
  static char TrackButton(size_t index) { return 0x90 + index; }

 private:
  void HandleInput();
  void ProcessInput(unsigned char* data, size_t data_size);
  void ProcessMessage();
  void SetButton(unsigned char button_value, ButtonState state);

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
  std::pair<unsigned char, bool> current_colors_[64];
  std::thread input_thread_;
  int signal_pipe_fd_[2];
  std::atomic<bool> running_ = false;
  constexpr static size_t kButtonQueueSize = 16;
  std::atomic<unsigned char> press_event_count_ = 0;
  std::atomic<unsigned char> press_event_buttons_[kButtonQueueSize];
  // There are two queues maintained to make sure that a button is never 'lost'.
  // When pressed, the button is added to both queues. When release, it is only
  // removed from the 'pressed' queue. Only when the application reads the
  // buttons, unpressed buttons are also removed from the 'queued' list.
  std::atomic<unsigned char> pressed_count_ = 0;
  std::atomic<unsigned char> pressed_buttons_[kButtonQueueSize];
  std::atomic<unsigned char> queued_count_ = 0;
  std::atomic<unsigned char> queued_buttons_[kButtonQueueSize];
  std::atomic<unsigned char> faders_[9];
};

}  // namespace glight::system::midi

#endif
