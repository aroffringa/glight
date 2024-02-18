#ifndef SYSTEM_MIDI_CONTROLLER_H_
#define SYSTEM_MIDI_CONTROLLER_H_

#include <algorithm>
#include <atomic>
#include <mutex>
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
    if (press_event_count_) {
      std::lock_guard lock(mutex_);
      button_set.SetSize(press_event_count_);
      std::copy_n(press_event_buttons_, button_set.Size(), button_set.Data());
      press_event_count_ = 0;
    } else {
      button_set.SetSize(0);
    }
  }

  void GetReleaseEvents(ButtonSet& button_set) {
    if (release_event_count_) {
      std::lock_guard lock(mutex_);
      button_set.SetSize(release_event_count_);
      std::copy_n(release_event_buttons_, button_set.Size(), button_set.Data());
      release_event_count_ = 0;
    } else {
      button_set.SetSize(0);
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
  static bool IsTrackButton(unsigned char button) {
    // 0x7A is shift
    return (button >= 0x64 && button < 0x6C) || button == 0x7A;
  }
  static size_t TrackButtonIndex(unsigned char button) {
    return button == 0x7A ? 8 : button - 0x64;
  }

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
  std::mutex mutex_;
  std::atomic<unsigned char> press_event_count_ = 0;
  unsigned char press_event_buttons_[kButtonQueueSize];
  std::atomic<unsigned char> release_event_count_ = 0;
  unsigned char release_event_buttons_[kButtonQueueSize];
  std::atomic<unsigned char> faders_[9];
};

}  // namespace glight::system::midi

#endif
