#ifndef SYSTEM_SMART_CONNECTION_H_
#define SYSTEM_SMART_CONNECTION_H_

#include <sigc++/connection.h>

namespace glight::system {

class SmartConnection {
 public:
  SmartConnection() noexcept = default;

  SmartConnection(SmartConnection&& source) noexcept {
    connection_ = std::move(source.connection_);
    source.connection_ = sigc::connection();
  }
  SmartConnection(const SmartConnection& source) = delete;

  explicit SmartConnection(sigc::connection&& connection) noexcept
      : connection_(connection) {}

  ~SmartConnection() noexcept { connection_.disconnect(); }

  SmartConnection& operator=(SmartConnection&& source) noexcept {
    connection_.disconnect();
    connection_ = std::move(source.connection_);
    source.connection_ = sigc::connection();
    return *this;
  }

  SmartConnection& operator=(const SmartConnection& source) = delete;
  SmartConnection& operator=(sigc::connection&& connection) noexcept {
    connection_.disconnect();
    connection_ = std::move(connection);
    return *this;
  }

  bool Empty() const noexcept { return connection_.empty(); }

  void Disconnect() noexcept { connection_.disconnect(); }

 private:
  sigc::connection connection_;
};

}  // namespace glight::system

#endif
