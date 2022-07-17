#ifndef THEATRE_DMXDEVICE_H_
#define THEATRE_DMXDEVICE_H_

#include <cstring>

namespace glight::theatre {

/**
        @author Andre Offringa
*/
class DmxDevice {
 public:
  DmxDevice() {}

  virtual ~DmxDevice() {}

  virtual void Open() = 0;

  virtual void SetValues(const unsigned char *newValues, size_t size) = 0;

  virtual void GetValues(unsigned char *destination, size_t size) = 0;

  virtual void WaitForNextSync() = 0;

  virtual void Abort() = 0;

  virtual bool IsOpen() = 0;
};

}  // namespace glight::theatre

#endif
