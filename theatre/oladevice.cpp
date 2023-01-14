#include "oladevice.h"

#include <ola/Logging.h>

#include <algorithm>
#include <iostream>
#include <memory>

#include <unistd.h>

namespace glight::theatre {

OLADevice::OLADevice() : _universe(1) {}

void OLADevice::Abort() {}

void OLADevice::Open() {
  // turn on OLA logging
  ola::InitLogging(ola::OLA_LOG_WARN, ola::OLA_LOG_STDERR);
  _buffer.Blackout();  // Set all channels to 0

  _client = std::make_unique<ola::client::StreamingClient>(
      (ola::client::StreamingClient::Options()));

  // Setup the client, this connects to the server
  if (!_client->Setup()) {
    throw std::runtime_error("Setup of OLA client failed");
  }
}

void OLADevice::SetValues(const unsigned char *newValues, size_t size) {
  size_t n = std::min<size_t>(512, size);
  for (size_t i = 0; i != n; ++i) {
    _buffer.SetChannel(i, newValues[i]);
  }
  if (!_client->SendDmx(_universe, _buffer)) {
    throw std::runtime_error("Sending OLA client DMX buffer failed");
  }
}

void OLADevice::GetValues(unsigned char *destination, size_t size) {
  size_t n = std::min<size_t>(512, size);
  for (size_t i = 0; i != n; ++i) {
    destination[i] = _buffer.Get(i);
  }
}

void OLADevice::WaitForNextSync() { usleep(40000); }

}  // namespace glight::theatre
