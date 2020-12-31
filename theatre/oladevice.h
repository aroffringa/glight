#ifndef OLA_DEVICE_H
#define OLA_DEVICE_H

#include "dmxdevice.h"

#include <ola/DmxBuffer.h>
#include <ola/client/StreamingClient.h>

#include <memory>

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

#endif
