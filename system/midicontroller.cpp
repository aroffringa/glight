#include "midicontroller.h"

#include <boost/algorithm/string/case_conv.hpp>

#include <unistd.h>

#include <fstream>
#include <stdexcept>

#include "jsonreader.h"

namespace glight::system {

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

std::vector<std::string> MidiController::DeviceNames() {
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
  const std::vector<std::string> names = MidiController::DeviceNames();
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

MidiController::MidiController(const std::string& device_name) {
  std::fill_n(current_colors_, 64, 0);
  color_map_ = ColorMap(ReadColorList(), 8);
  Check(snd_rawmidi_open(&in_rmidi_, &out_rmidi_, device_name.c_str(), 0));
  Check(snd_rawmidi_nonblock(out_rmidi_, 1 /*non-block*/));
  running_ = true;
  if (pipe(signal_pipe_fd_) < 0)
    throw std::runtime_error("Failed to create pipe");
  input_thread_ = std::thread([&]() { HandleInput(); });
  /*
  for(unsigned char pad = 0; pad!=0x40; ++pad) {
    const unsigned char buffer[] = {0x96, pad, 0x20};
    Check(snd_rawmidi_write(out_rmidi_, buffer, 3));
  }
  const unsigned char full_on = 0x1;
  const unsigned char blink = 0x2;
  for(unsigned char track_button = 0x64; track_button!=0x6C; ++track_button) {
    const unsigned char buffer[] = {0x90, track_button, full_on};
    Check(snd_rawmidi_write(out_rmidi_, buffer, 3));
  }
  for(unsigned char scene_button = 0x70; scene_button!=0x78; ++scene_button) {
    const unsigned char buffer[] = {0x90, scene_button, full_on};
    Check(snd_rawmidi_write(out_rmidi_, buffer, 3));
  }
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
  for (size_t i = 0; i != 8; ++i) {
    SetPixelColor(i, i, Color::Orange(), false);
    SetPixelColor(7 - i, i, Color::Purple(), false);
  }*/
  const std::vector<Color> colors = Color::DefaultSet32();
  for (size_t i = 0; i != 32; ++i) {
    SetPixelColor(i / 8, i % 8, colors[i], false);
    SetPixelColor(i / 8 + 4, i % 8, colors[i], false);
  }
}

MidiController::~MidiController() noexcept {
  running_ = false;
  unsigned char signal_buffer = 0;
  write(signal_pipe_fd_[1], &signal_buffer, 1);
  input_thread_.join();
  close(signal_pipe_fd_[1]);
  close(signal_pipe_fd_[0]);
  snd_rawmidi_close(in_rmidi_);
  snd_rawmidi_close(out_rmidi_);
}

std::optional<MidiController> MidiController::GetController() {
  std::vector<Device> devices = DeviceList();
  std::vector<Device>::iterator apc_mini_mk2 =
      std::find_if(devices.begin(), devices.end(), [](const Device& device) {
        return boost::to_lower_copy(device.name) == "apc mini mk2";
      });

  if (apc_mini_mk2 != devices.end()) {
    return make_optional<MidiController>(apc_mini_mk2->device);
  } else {
    return {};
  }
}

void MidiController::SetPixelColor(size_t column, size_t row,
                                   const theatre::Color& color, bool blink) {
  const unsigned char color_index = color_map_.GetIndex(color);
  const unsigned char pad = column + row * 8;
  if (current_colors_[pad] != color_index) {
    constexpr unsigned char pad_full = 0x96;
    constexpr unsigned char pad_blink = 0x9F;
    const unsigned char buffer[] = {blink ? pad_blink : pad_full, pad,
                                    color_index};
    // Return value is unchecked: errors are explicitly ignored
    snd_rawmidi_write(out_rmidi_, buffer, 3);

    current_colors_[pad] = color_index;
  }
}

void MidiController::HandleInput() {
  input_state_ = InputState::Empty;
  pressed_count_ = 0;
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

void MidiController::ProcessInput(unsigned char* data, size_t data_size) {
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

void AddButton(unsigned char button, std::atomic<unsigned char>* list,
               size_t list_size, std::atomic<unsigned char>& button_count) {
  for (size_t i = 0; i != button_count; ++i) {
    if (list[i] == button) return;
  }
  if (button_count < list_size) {
    list[button_count] = button;
    ++button_count;
  }
}

void RemoveButton(unsigned char button, std::atomic<unsigned char>* list,
                  std::atomic<unsigned char>& button_count) {
  for (size_t i = 0; i != button_count; ++i) {
    if (list[i] == button) {
      for (size_t j = i; j != button_count - 1u; ++j) {
        list[j] = list[j + 1].load();
      }
      --button_count;
      return;
    }
  }
}

void MidiController::ProcessMessage() {
  switch (input_state_) {
    case InputState::NoteOn:
      AddButton(input_data_[1], pressed_buttons_, std::size(pressed_buttons_),
                pressed_count_);
      AddButton(input_data_[1], queued_buttons_, std::size(queued_buttons_),
                queued_count_);
      break;
    case InputState::NoteOff:
      RemoveButton(input_data_[1], pressed_buttons_, pressed_count_);
      break;
    case InputState::Controller: {
      size_t fader = std::max<unsigned char>(48, input_data_[1]) - 48;
      faders_[fader] = input_data_[2];
    } break;
    case InputState::Empty:
      break;
  }
}

}  // namespace glight::system
