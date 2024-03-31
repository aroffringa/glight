#include "olaconnection.h"

#include <ola/Logging.h>

#include <algorithm>
#include <iostream>
#include <memory>

namespace glight::theatre {

OlaConnection::OlaConnection() = default;

void OlaConnection::Abort() {
  abort_ = true;
  ola_thread_.join();
}

void OlaConnection::Open() {
  abort_ = false;

  // turn on OLA logging
  ola::InitLogging(ola::OLA_LOG_WARN, ola::OLA_LOG_STDERR);

  client_ = std::make_unique<ola::client::OlaClientWrapper>();

  // Setup the client, this connects to the server
  if (!client_->Setup()) {
    throw std::runtime_error("Setup of OLA client failed");
  }
  client_->GetClient()->FetchUniverseList(
      ola::NewSingleCallback(this, &OlaConnection::ReceiveUniverseList));
  client_->GetSelectServer()->Run();
  ola_thread_ = std::thread([&]() { client_->GetSelectServer()->Run(); });
}

bool OlaConnection::SendDmx() {
  for (const std::pair<const size_t, OlaUniverse>& u : universes_) {
    const OlaUniverse& ola_universe = u.second;
    if (ola_universe.type == UniverseType::Output) {
      client_->GetClient()->SendDMX(u.first, *ola_universe.send_buffer,
                                    send_dmx_args_);
    }
  }
  if (abort_) client_->GetSelectServer()->Terminate();
  send_event_.Release();
  return true;
}

void OlaConnection::SetOutputValues(unsigned universe,
                                    const unsigned char* newValues,
                                    size_t size) {
  const size_t n = std::min<size_t>(512, size);
  std::lock_guard<std::mutex> lock(receive_mutex_);
  OlaUniverse& ola_universe = universes_[universe];
  if (ola_universe.type == UniverseType::Uninitialized) {
    ola_universe.type = UniverseType::Output;
    ola_universe.send_buffer.emplace();
    ola_universe.send_buffer->Blackout();  // Set all channels to 0
  }
  ola::DmxBuffer& buffer = *ola_universe.send_buffer;
  for (size_t i = 0; i != n; ++i) {
    buffer.SetChannel(i, newValues[i]);
  }
}

void OlaConnection::GetOutputValues(unsigned universe,
                                    unsigned char* destination, size_t size) {
  const size_t n = std::min<size_t>(512, size);
  std::lock_guard<std::mutex> lock(receive_mutex_);
  if (universe < universes_.size()) {
    const OlaUniverse& ola_universe = universes_[universe];
    if (ola_universe.type == UniverseType::Output) {
      const ola::DmxBuffer& buffer = *ola_universe.send_buffer;
      for (size_t i = 0; i != n; ++i) {
        destination[i] = buffer.Get(i);
      }
    }
  }
}

void OlaConnection::GetInputValues(unsigned universe,
                                   unsigned char* destination, size_t size) {
  std::lock_guard<std::mutex> lock(receive_mutex_);
  const OlaUniverse& ola_universe = universes_.find(universe)->second;
  if (ola_universe.type == UniverseType::Input) {
    const std::vector<unsigned char>& buffer = ola_universe.receive_buffer;
    std::copy_n(buffer.data(), std::min(buffer.size(), size), destination);
  } else {
    std::fill_n(destination, size, 0);
  }
}

void OlaConnection::WaitForNextSync() { send_event_.Wait(); }

void OlaConnection::ReceiveDmx(const ola::client::DMXMetadata& metadata,
                               const ola::DmxBuffer& data) {
  std::lock_guard<std::mutex> lock(receive_mutex_);
  const unsigned universe = metadata.universe;
  std::vector<unsigned char>& buffer = universes_[universe].receive_buffer;
  std::copy_n(data.GetRaw(), std::min<unsigned>(data.Size(), buffer.size()),
              buffer.data());
}

void OlaConnection::RegisterUniverseCallback(
    const ola::client::Result& result) {}

void OlaConnection::ReceiveUniverseList(
    const ola::client::Result& result,
    const std::vector<ola::client::OlaUniverse>& universes) {
  for (const ola::client::OlaUniverse& u : universes) {
    OlaUniverse& ola_universe = universes_[u.Id()];
    if (u.InputPortCount() != 0) {
      std::cout << "Input universe " << u.Id() << ": " << u.Name() << '\n';
      ola_universe.type = UniverseType::Input;
      ola_universe.receive_buffer.resize(512);
      client_->GetClient()->RegisterUniverse(
          u.Id(), ola::client::REGISTER,
          ola::NewSingleCallback(this,
                                 &OlaConnection::RegisterUniverseCallback));
    } else {
      std::cout << "Output universe " << u.Id() << ": " << u.Name() << '\n';
      ola_universe.type = UniverseType::Output;
      ola_universe.send_buffer.emplace();
    }
  }
  if (universes_.empty()) throw std::runtime_error("No ola universes defined");

  // Enable DMX send/receive callbacks
  client_->GetClient()->SetDMXCallback(
      ola::NewCallback(this, &OlaConnection::ReceiveDmx));
  ola::io::SelectServer* ss = client_->GetSelectServer();
  ss->RegisterRepeatingTimeout(25,
                               ola::NewCallback(this, &OlaConnection::SendDmx));
  ss->Terminate();
}

}  // namespace glight::theatre
