#ifndef THEATRE_OLA_DEVICE_H_
#define THEATRE_OLA_DEVICE_H_

#include "dmxdevice.h"

#include "../system/event_synchronization.h"

#include <ola/DmxBuffer.h>
#include <ola/client/ClientWrapper.h>

#include <optional>
#include <memory>
#include <semaphore>

namespace glight::theatre {

enum class OlaUniverseType { Uninitialized, Output, Input };

struct OlaUniverse {
  OlaUniverseType type = OlaUniverseType::Uninitialized;
  std::optional<ola::DmxBuffer> send_buffer;
  std::vector<unsigned char> receive_buffer;
};

class OLADevice final : public DmxDevice {
 public:
  OLADevice();

  void Open() override;
  size_t NOutputUniverses() const override {
    return std::count_if(universes_.begin(), universes_.end(),
                         [](const OlaUniverse &universe) {
                           return universe.type == OlaUniverseType::Output;
                         });
  }
  void SetOutputValues(unsigned universe, const unsigned char *newValues,
                       size_t size) override;
  void GetOutputValues(unsigned universe, unsigned char *destination,
                       size_t size) override;
  void GetInputValues(unsigned universe, unsigned char *destination,
                      size_t size) override;
  void WaitForNextSync() override;
  void Abort() override;
  bool IsOpen() override { return true; }

 private:
  void ReceiveDmx(const ola::client::DMXMetadata &metadata,
                  const ola::DmxBuffer &data);
  void ReceiveUniverseList(
      const ola::client::Result &result,
      const std::vector<ola::client::OlaUniverse> &universes);
  bool SendDmx();
  void RegisterUniverseCallback(const ola::client::Result &result);

  std::mutex send_mutex_;
  std::mutex receive_mutex_;
  system::EventSynchronization send_event_;

  std::vector<OlaUniverse> universes_;
  std::unique_ptr<ola::client::OlaClientWrapper> client_;
  ola::client::SendDMXArgs send_dmx_args_;
  std::thread ola_thread_;
  std::atomic<bool> abort_ = false;
};

}  // namespace glight::theatre

#endif
