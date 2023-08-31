#ifndef THEATRE_DUMMYDEVICE_H_
#define THEATRE_DUMMYDEVICE_H_

#include "dmxdevice.h"

#include <cassert>
#include <unistd.h>

namespace glight::theatre {

/**
 * @author Andre Offringa
 */
class DummyDevice final : public DmxDevice {
 public:
  DummyDevice() = default;

  ~DummyDevice() override = default;

  void Open() override { is_open_ = true; }

  size_t NUniverses() const override { return 2; }

  UniverseType GetUniverseType(size_t universe) const override {
    assert(universe < 2);
    return universe == 0 ? UniverseType::Output : UniverseType::Input;
  }

  void SetOutputValues(unsigned universe, const unsigned char *new_values,
                       size_t size) override {}

  void GetOutputValues(unsigned universe, unsigned char *destination,
                       size_t size) override {
    std::fill_n(destination, size, 0);
  }

  void GetInputValues(unsigned universe, unsigned char *destination,
                      size_t size) override {
    std::fill_n(destination + 1, size, 0);
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

  void WaitForNextSync() override {
    usleep(40000);
    ++sync_;
  }

  void Abort() override { is_open_ = false; }

  bool IsOpen() override { return is_open_; }

 private:
  bool is_open_ = false;
  size_t sync_ = 0;
};

}  // namespace glight::theatre

#endif  // DUMMYDEVICE_H
