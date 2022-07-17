#ifndef THEATRE_DUMMYDEVICE_H_
#define THEATRE_DUMMYDEVICE_H_

#include "dmxdevice.h"

#include <unistd.h>

namespace glight::theatre {

/**
        @author Andre Offringa
*/
class DummyDevice : public DmxDevice {
 public:
  DummyDevice() : _isOpen(false) {}

  virtual ~DummyDevice() {}

  virtual void Open() final override { _isOpen = true; }

  virtual void SetValues(const unsigned char *newValues,
                         size_t size) final override {}

  virtual void GetValues(unsigned char *destination,
                         size_t size) final override {
    for (size_t i = 0; i < size; ++i) destination[i] = 0;
  }

  virtual void WaitForNextSync() final override { usleep(40000); }

  virtual void Abort() final override { _isOpen = false; }

  virtual bool IsOpen() final override { return _isOpen; }

 private:
  bool _isOpen;
};

}  // namespace glight::theatre

#endif  // DUMMYDEVICE_H
