#ifndef THEATRE_DUMMYDEVICE_H_
#define THEATRE_DUMMYDEVICE_H_

#include "dmxdevice.h"

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

  size_t NOutputUniverses() const override { return 1; }

  void SetOutputValues(unsigned universe, const unsigned char *new_values,
                       size_t size) override {}

  void GetOutputValues(unsigned universe, unsigned char *destination,
                       size_t size) override {
    std::fill_n(destination, size, 0);
  }

  void GetInputValues(unsigned universe, unsigned char *destination,
                      size_t size) override {
    std::fill_n(destination + 1, size, 0);
    if (universe == 1)
      *destination = 255.0 * 0.5 * (std::cos(-sync_ * M_PI / 25.0) + 0.5);
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
