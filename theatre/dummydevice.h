#ifndef THEATRE_DUMMYDEVICE_H_
#define THEATRE_DUMMYDEVICE_H_

#include "dmxdevice.h"

#include <unistd.h>

namespace glight::theatre {

/**
        @author Andre Offringa
*/
class DummyDevice final : public DmxDevice {
 public:
  DummyDevice() = default;

  ~DummyDevice() override = default;

  void Open() override { _isOpen = true; }

  size_t NOutputUniverses() const override { return 1; }

  void SetOutputValues(unsigned universe, const unsigned char *newValues,
                       size_t size) override {}

  void GetOutputValues(unsigned universe, unsigned char *destination,
                       size_t size) override {
    std::fill_n(destination, size, 0);
  }

  void GetInputValues(unsigned universe, unsigned char *destination,
                      size_t size) override {
    std::fill_n(destination, size, 0);
  }

  void WaitForNextSync() override { usleep(40000); }

  void Abort() override { _isOpen = false; }

  bool IsOpen() override { return _isOpen; }

 private:
  bool _isOpen = false;
};

}  // namespace glight::theatre

#endif  // DUMMYDEVICE_H
