#ifndef GLIGHT_GUI_CONNECTION_MANAGER_H_
#define GLIGHT_GUI_CONNECTION_MANAGER_H_

#include <vector>

#include <sigc++/connection.h>

/**
 * Class that will automatically disconnect connections when destructed.
 */
class ConnectionManager {
 public:
  ConnectionManager() noexcept = default;
  ~ConnectionManager() {
    for (sigc::connection& connection : connections_) {
      connection.disconnect();
    }
  }

  void Add(sigc::connection&& connection) {
    connections_.emplace_back(std::move(connection));
  }

 private:
  std::vector<sigc::connection> connections_;
};

#endif
