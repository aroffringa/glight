#ifndef SYSTEM_READ_H_
#define SYSTEM_READ_H_

#include <iostream>
#include <string>

namespace glight {

namespace gui {
class GUIState;
}
namespace theatre {
class Management;
}

namespace system {

void Read(const std::string &filename, theatre::Management &management,
          gui::GUIState *guiState = nullptr);
void Read(std::istream &stream, theatre::Management &management,
          gui::GUIState *guiState = nullptr);
}  // namespace system
}  // namespace glight

#endif
