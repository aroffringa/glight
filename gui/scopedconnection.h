#ifndef GLIGHT_GUI_SCOPED_CONNECTION_H_
#define GLIGHT_GUI_SCOPED_CONNECTION_H_

#include <sigc++/connection.h>

namespace glight::gui {

/**
 * Class that will automatically disconnect a single connection when destructed.
 */
class ScopedConnection {
 public:
  ScopedConnection() noexcept = default;
  ScopedConnection(sigc::connection&& sigc_connection) noexcept
      : connection_(std::move(sigc_connection)) {}
  ScopedConnection(const ScopedConnection&) = delete;
  ScopedConnection(ScopedConnection&& rhs) noexcept {
    connection_ = std::move(rhs.connection_);
    rhs.connection_ = sigc::connection();
  }

  ~ScopedConnection() noexcept { connection_.disconnect(); }
  ScopedConnection& operator=(const ScopedConnection&) = delete;
  ScopedConnection& operator=(ScopedConnection&& rhs) noexcept {
    connection_ = std::move(rhs.connection_);
    rhs.connection_ = sigc::connection();
    return *this;
  }
  ScopedConnection& operator=(sigc::connection&& sigc_connection) noexcept {
    connection_.disconnect();
    connection_ = std::move(sigc_connection);
    return *this;
  }

 private:
  sigc::connection connection_;
};

}  // namespace glight::gui

#endif
