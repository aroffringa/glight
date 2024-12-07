#include "controller.h"

#include <boost/algorithm/string/case_conv.hpp>

#include <unistd.h>

#include <fstream>
#include <stdexcept>

#include "system/jsonreader.h"

namespace glight::system::midi {

using theatre::Color;

namespace {
void Check(int result) {
  if (result < 0) {
    const char* error_str = snd_strerror(result);
    throw std::runtime_error(
        std::string("Alsa raw midi interface returned an error: ") + error_str);
  }
}

std::string CopyAndFree(char* input) {
  std::string result = input;
  free(input);
  return result;
}

std::string CheckedCopy(const char* str) {
  if (str)
    return std::string(str);
  else
    return {};
}

unsigned char FromHex(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'A' && c <= 'F')
    return c - ('A' - 10);
  else if (c >= 'a' && c <= 'f')
    return c - ('a' - 10);
  else
    throw std::runtime_error("Invalid symbol in hexademical string");
}

std::vector<Color> ReadColorList() {
  std::ifstream file("../data/controllers/apc-mini-mk2.json");
  std::vector<Color> colors;
  if (file) {
    std::unique_ptr<json::Node> root = glight::json::Parse(file);
    const glight::json::Object& obj = ToObj(*root);
    const glight::json::Array& colors_node = ToArr(obj["colors"]);
    for (glight::json::Node& color_node : colors_node) {
      const std::string& color_str = ToStr(color_node);
      if (color_str.size() != 6)
        throw std::runtime_error(
            "Invalid color string: should contain 6 characters");
      const unsigned char red =
          (FromHex(color_str[0]) << 4) | FromHex(color_str[1]);
      const unsigned char green =
          (FromHex(color_str[2]) << 4) | FromHex(color_str[3]);
      const unsigned char blue =
          (FromHex(color_str[4]) << 4) | FromHex(color_str[5]);
      colors.emplace_back(red, green, blue);
    }
  }
  return colors;
}

struct Device {
  std::string device;
  std::string name;
};

}  // namespace

std::vector<std::string> Controller::DeviceNames() {
  void** names;
  Check(snd_device_name_hint(-1, "rawmidi", &names));
  void** name_iterator = names;
  std::vector<std::string> devices;
  while (*name_iterator != nullptr) {
    devices.emplace_back(
        CopyAndFree(snd_device_name_get_hint(*name_iterator, "NAME")));
    ++name_iterator;
  }
  snd_device_name_free_hint(names);
  return devices;
}

std::vector<Device> DeviceList() {
  std::vector<Device> devices;
  const std::vector<std::string> names = Controller::DeviceNames();
  for (const std::string& device_name : names) {
    try {
      Device device;
      device.device = device_name;
      snd_rawmidi_t* in_rmidi;
      Check(snd_rawmidi_open(&in_rmidi, nullptr, device_name.c_str(), 0));

      snd_rawmidi_info_t* in_info;
      Check(snd_rawmidi_info_malloc(&in_info));
      const int result = snd_rawmidi_info(in_rmidi, in_info);
      device.name = CheckedCopy(snd_rawmidi_info_get_name(in_info));
      snd_rawmidi_info_free(in_info);
      Check(result);

      snd_rawmidi_close(in_rmidi);

      devices.emplace_back(device);
    } catch (std::exception& e) {
      std::cerr << "Ignoring midi device '" << device_name
                << "' because of alsa error:\n"
                << e.what() << '\n';
    }
  }
  return devices;
}

Controller::Controller(const std::string& device_name) {
  std::fill_n(current_colors_, 64, std::pair((unsigned char)(0), false));
  color_map_ = ColorMap(ReadColorList(), 8);
  Check(snd_rawmidi_open(&in_rmidi_, &out_rmidi_, device_name.c_str(), 0));
  Check(snd_rawmidi_nonblock(out_rmidi_, 1 /*non-block*/));
  running_ = true;
  if (pipe(signal_pipe_fd_) < 0)
    throw std::runtime_error("Failed to create pipe");
  input_thread_ = std::thread([&]() { HandleInput(); });
  /*
  for(unsigned char red =0; red!=0x78; red+=8) {
    std::vector<unsigned char> buffer{0xF0, 0x47, 0x7F, 0x4F, 0x24,
      0x0, 0x8, // nbytes to follow
      0x010, 0x1f, //pad range
      0x00, red, // red
      0x00, 0x00, // green
      0x00, 0x00, // blue
      0xF7}; // termination symbol
    Check(snd_rawmidi_write(out_rmidi_, buffer.data(), buffer.size()));
  }
*/
}

Controller::~Controller() noexcept {
  running_ = false;
  unsigned char signal_buffer = 0;
  [[maybe_unused]] ssize_t write_result = write(signal_pipe_fd_[1], &signal_buffer, 1);
  input_thread_.join();
  close(signal_pipe_fd_[1]);
  close(signal_pipe_fd_[0]);
  snd_rawmidi_close(in_rmidi_);
  snd_rawmidi_close(out_rmidi_);
}

std::unique_ptr<Controller> Controller::GetController() {
  std::vector<Device> devices = DeviceList();
  std::vector<Device>::iterator apc_mini_mk2 =
      std::find_if(devices.begin(), devices.end(), [](const Device& device) {
        return boost::to_lower_copy(device.name) == "apc mini mk2";
      });

  if (apc_mini_mk2 != devices.end()) {
    return make_unique<Controller>(apc_mini_mk2->device);
  } else {
    return {};
  }
}

void Controller::SetPixelColor(size_t column, size_t row,
                               const theatre::Color& color, bool blink) {
  const unsigned char color_index = color_map_.GetIndex(color);
  const unsigned char pad = column + row * 8;
  if (current_colors_[pad] != std::pair(color_index, blink)) {
    constexpr unsigned char pad_full = 0x96;
    constexpr unsigned char pad_blink = 0x9F;
    const unsigned char buffer[] = {blink ? pad_blink : pad_full, pad,
                                    color_index};
    // Return value is unchecked: errors are explicitly ignored
    snd_rawmidi_write(out_rmidi_, buffer, 3);

    current_colors_[pad] = std::pair(color_index, blink);
  }
}

void Controller::HandleInput() {
  input_state_ = InputState::Empty;
  press_event_count_ = 0;
  release_event_count_ = 0;
  std::fill_n(faders_, 9, 0);

  const unsigned n_pfds = snd_rawmidi_poll_descriptors_count(in_rmidi_);
  std::vector<pollfd> pfds(n_pfds);
  snd_rawmidi_poll_descriptors(in_rmidi_, pfds.data(), n_pfds);
  pollfd signal_fd;
  signal_fd.events = POLLIN;
  signal_fd.fd = signal_pipe_fd_[0];
  pfds.emplace_back(signal_fd);
  bool error = false;
  std::vector<unsigned char> buffer(256);
  while (running_ && !error) {
    const int poll_result = poll(pfds.data(), pfds.size(), -1);
    if (poll_result < 0) {
      error = true;
    } else if (poll_result > 0) {
      const ssize_t read_size =
          snd_rawmidi_read(in_rmidi_, buffer.data(), buffer.size());
      if (read_size > 0) {
        ProcessInput(buffer.data(), read_size);
      }
    }
  }
}

void Controller::ProcessInput(unsigned char* data, size_t data_size) {
  for (size_t i = 0; i != data_size; ++i) {
    switch (input_state_) {
      case InputState::Empty:
        input_data_[0] = data[i];
        input_size_ = 1;
        if ((data[i] & 0xF0) == 0x90) {
          input_state_ = InputState::NoteOn;
        } else if ((data[i] & 0xF0) == 0x80) {
          input_state_ = InputState::NoteOff;
        } else if ((data[i] & 0xF0) == 0xB0) {
          input_state_ = InputState::Controller;
        }
        break;
      case InputState::NoteOn:
      case InputState::NoteOff:
      case InputState::Controller:
        input_data_[input_size_] = data[i];
        ++input_size_;
        if (input_size_ == 3) {
          ProcessMessage();
          input_size_ = 0;
          input_state_ = InputState::Empty;
        }
        break;
    }
  }
}

void AddButton(unsigned char button, unsigned char* list, size_t list_size,
               std::atomic<unsigned char>& button_count) {
  for (size_t i = 0; i != button_count; ++i) {
    if (list[i] == button) return;
  }
  if (button_count < list_size) {
    list[button_count] = button;
    ++button_count;
  }
}

void Controller::ProcessMessage() {
  switch (input_state_) {
    case InputState::NoteOn: {
      std::scoped_lock lock(mutex_);
      AddButton(input_data_[1], press_event_buttons_,
                std::size(press_event_buttons_), press_event_count_);
    } break;
    case InputState::NoteOff: {
      std::scoped_lock lock(mutex_);
      AddButton(input_data_[1], release_event_buttons_,
                std::size(release_event_buttons_), release_event_count_);
    } break;
    case InputState::Controller: {
      size_t fader = std::max<unsigned char>(48, input_data_[1]) - 48;
      faders_[fader] = input_data_[2];
    } break;
    case InputState::Empty:
      break;
  }
}

void Controller::SetTrackButton(size_t index, ButtonState state) {
  if (index < 8) {
    SetButton(0x64 + index, state);
  }
}

void Controller::SetSceneButton(size_t index, ButtonState state) {
  if (index < 8) {
    SetButton(0x70 + index, state);
  }
}

void Controller::SetButton(unsigned char button_value, ButtonState state) {
  constexpr unsigned char full_off = 0x0;
  constexpr unsigned char full_on = 0x1;
  constexpr unsigned char blink = 0x2;
  unsigned char state_value = full_off;
  switch (state) {
    case ButtonState::Off:
      state_value = full_off;
      break;
    case ButtonState::On:
      state_value = full_on;
      break;
    case ButtonState::Blink:
      state_value = blink;
      break;
  }
  const unsigned char buffer[] = {0x90, button_value, state_value};
  Check(snd_rawmidi_write(out_rmidi_, buffer, 3));
}

}  // namespace glight::system::midi
