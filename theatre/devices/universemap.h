#ifndef THEATRE_DEVICES_UNIVERSE_MAP_H_
#define THEATRE_DEVICES_UNIVERSE_MAP_H_

#include "theatre/devices/olaconnection.h"

#include <unistd.h>  // usleep

#include <cmath>
#include <variant>

#include "system/optionalnumber.h"

namespace glight::theatre::devices {

enum class InputMappingFunction {
  NoFunction,
  /// This input controls the faders of a fader window
  FaderControl,
  /// The input channels are merged into the output.
  Merge
};

struct InputMapping {
  /// Function of this mapping.
  InputMappingFunction function;
  system::OptionalNumber<size_t> merge_universe;
  /// Ola universe index of this mapping. If unset, it is a dummy input.
  system::OptionalNumber<size_t> ola_universe;
};

struct OutputMapping {
  /// Ola universe index of this mapping. If unset, it is a dummy output.
  system::OptionalNumber<size_t> ola_universe;
};

using UniverseMapping = std::variant<InputMapping, OutputMapping>;

class UniverseMap {
 public:
  UniverseMap()
      : mappings_{OutputMapping(),
                  InputMapping{InputMappingFunction::NoFunction, {}, {}}} {}

  /**
   * The reason for a separate Open() function instead of handling this in
   * the constructor, is to also have an "empty" UniverseMap, which can be
   * move from/to result.
   *
   * If called when already open, Ola will be closed and re-opened, which
   * will cause the universe mapping to reset to a default mapping to the
   * available Ola universes.
   */
  void Open();

  void Close() {
    if (ola_) {
      ola_->Abort();
      ola_.reset();
    }
  }

  size_t NUniverses() const { return mappings_.size(); }

  void SetUniverseMapping(size_t universe, UniverseMapping& mapping) {
    mappings_[universe] = mapping;
  }
  UniverseType GetUniverseType(size_t universe) const {
    return std::holds_alternative<InputMapping>(mappings_[universe])
               ? UniverseType::Input
               : UniverseType::Output;
  }
  const InputMapping& GetInputMapping(size_t universe) const {
    return std::get<InputMapping>(mappings_[universe]);
  }
  const OutputMapping& GetOutputMapping(size_t universe) const {
    return std::get<OutputMapping>(mappings_[universe]);
  }
  const UniverseMapping& GetMapping(size_t universe) const {
    return mappings_[universe];
  }
  unsigned FirstOutputUniverse() const {
    for (size_t i = 0; i != mappings_.size(); ++i) {
      if (std::holds_alternative<OutputMapping>(mappings_[i])) return i;
    }
    return 0;
  }

  void SetOutputValues(unsigned universe, const unsigned char* new_values,
                       size_t size) {
    const OutputMapping& mapping = std::get<OutputMapping>(mappings_[universe]);
    if (mapping.ola_universe) {
      ola_->SetOutputValues(*mapping.ola_universe, new_values, size);
    }
  }

  void GetOutputValues(unsigned universe, unsigned char* destination,
                       size_t size) {
    const OutputMapping& mapping = std::get<OutputMapping>(mappings_[universe]);
    if (mapping.ola_universe) {
      ola_->GetOutputValues(*mapping.ola_universe, destination, size);
    }
  }

  void GetInputValues(unsigned universe, unsigned char* destination,
                      size_t size) {
    const InputMapping* mapping =
        std::get_if<InputMapping>(&mappings_[universe]);
    if (mapping && mapping->ola_universe) {
      ola_->GetInputValues(*mapping->ola_universe, destination, size);
    } else {
      // This is a dummy input ; generate a sinusoid
      std::fill_n(destination + 1, size - 1, 0);
      if (universe == 1) {
        if (sync_ >= 150) {
          sync_ = 0;
        }
        if (sync_ >= 75 && sync_ < 125) {
          *destination = 0;
        } else if (sync_ >= 50) {
          const float float_value =
              (255.0 * 0.5) *
              (std::cos(-static_cast<float>(sync_) * M_PI / 25.0) + 1.0);
          *destination = std::round(float_value);
        } else {
          *destination = 255;
        }
      }
    }
  }

  void WaitForNextSync() {
    if (ola_)
      ola_->WaitForNextSync();
    else
      usleep(40000);
    ++sync_;
  }

  const std::unique_ptr<OlaConnection>& GetOla() const { return ola_; }

 private:
  std::vector<UniverseMapping> mappings_;
  std::unique_ptr<OlaConnection> ola_;
  size_t sync_ = 0;
};

}  // namespace glight::theatre::devices

#endif
