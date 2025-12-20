#ifndef SYSTEM_WRITER_H_
#define SYSTEM_WRITER_H_

#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include "jsonwriter.h"

namespace glight {

namespace uistate {
class UIState;
}

namespace theatre {
class Management;
}

namespace system {

class WriterException : public std::runtime_error {
 public:
  WriterException(const std::string &msg) : std::runtime_error(msg) {}
};

void Write(std::ostream &stream, theatre::Management &management,
           uistate::UIState *uiState);
void Write(const std::string &filename, theatre::Management &management,
           uistate::UIState *uiState);

}  // namespace system
}  // namespace glight

#endif
