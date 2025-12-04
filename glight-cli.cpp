#include <iostream>
#include <string>

#include "system/reader.h"
#include "system/settings.h"
#include "theatre/management.h"

namespace glight {

void RunCli(const std::string filename) {
  const glight::system::Settings settings = glight::system::LoadSettings();
  glight::theatre::Management management(settings);
  glight::system::Read(filename, management);
  management.GetUniverses().Open();
  management.Run();
  std::cout << "Press enter to exit.\n";
  std::cin.get();
  management.BlackOut(false, 0.0f);
  std::cout << "Stopping...\n";
  // There is some time required for the black out to take effect.
  usleep(400000);
}

}  // namespace glight

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    std::cout << "Syntax: glight-cli <show-file>\n\n"
                 "glight-cli can output a previously created gshow file "
                 "without requiring a graphical desktop.\n";
    return 0;
  }

  glight::RunCli(argv[1]);
}
