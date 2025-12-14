#ifndef SYSTEM_READ_H_
#define SYSTEM_READ_H_

#include <iostream>
#include <string>

namespace glight {

namespace uistate {
class UIState;
}

namespace theatre {
class Management;
}

namespace system {

void ImportFixtureTypes(const std::string &filename,
                        theatre::Management &management);

void Read(const std::string &filename, theatre::Management &management,
          uistate::UIState *ui_state = nullptr);
void Read(std::istream &stream, theatre::Management &management,
          uistate::UIState *ui_state = nullptr);
}  // namespace system
}  // namespace glight

#endif
