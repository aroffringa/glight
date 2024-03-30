#ifndef THEATRE_OLA_CONNECTION_H_
#define THEATRE_OLA_CONNECTION_H_

#include "system/event_synchronization.h"

#include <ola/DmxBuffer.h>
#include <ola/client/ClientWrapper.h>

#include <cassert>
#include <optional>
#include <memory>
#include <semaphore>

namespace glight::theatre {

enum class UniverseType { Uninitialized, Output, Input };

struct OlaUniverse {
  UniverseType type = UniverseType::Uninitialized;
  std::optional<ola::DmxBuffer> send_buffer;
  std::vector<unsigned char> receive_buffer;
};

class OlaConnection {
 public:
  OlaConnection();

  void Open();
  size_t NUniverses() const { return universes_.size(); }
  UniverseType GetUniverseType(size_t universe) const {
    return universes_[universe].type;
  }
  void SetOutputValues(unsigned universe, const unsigned char *newValues,
                       size_t size);
  void GetOutputValues(unsigned universe, unsigned char *destination,
                       size_t size);
  void GetInputValues(unsigned universe, unsigned char *destination,
                      size_t size);
  void WaitForNextSync();
  void Abort();

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
