#include "midicontroller.h"

#include <stdexcept>

#include <boost/algorithm/string/case_conv.hpp>

#include <fstream>
#include <iostream>

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

MidiController::MidiController() {
  std::fill_n(current_colors_, 64, 0);
  color_map_ = ColorMap(ReadColorList(), 8);
  std::vector<Device> devices = DeviceList();
  std::vector<Device>::iterator apc_mini_mk2 =
      std::find_if(devices.begin(), devices.end(), [](const Device& device) {
        return boost::to_lower_copy(device.name) == "apc mini mk2";
      });

  if (apc_mini_mk2 != devices.end()) {
    Check(snd_rawmidi_open(&in_rmidi_, &out_rmidi_,
                           apc_mini_mk2->device.c_str(), 0));

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
    }*/
  }
  for (size_t i = 0; i != 8; ++i) {
    SetPixelColor(i, i, Color::Yellow(), false);
    SetPixelColor(7 - i, i, Color::Purple(), false);
  }
}

MidiController::~MidiController() noexcept {
  snd_rawmidi_close(in_rmidi_);
  snd_rawmidi_close(out_rmidi_);
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

}  // namespace glight::system
