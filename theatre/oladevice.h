#ifndef THEATRE_OLA_DEVICE_H_
#define THEATRE_OLA_DEVICE_H_

#include "dmxdevice.h"

#include <ola/DmxBuffer.h>
#include <ola/client/StreamingClient.h>

#include <memory>

namespace glight::theatre {

class OLADevice : public DmxDevice {
 public:
  OLADevice();

  void Open() final override;
  void SetValues(const unsigned char *newValues, size_t size) final override;
  void GetValues(unsigned char *destination, size_t size) final override;
  void WaitForNextSync() final override;
  void Abort() final override;
  bool IsOpen() final override { return true; }

 private:
  ola::DmxBuffer _buffer;
  std::unique_ptr<ola::client::StreamingClient> _client;
  unsigned int _universe;
};

}  // namespace glight::theatre

#endif
