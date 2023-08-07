#include <cstdlib>
#include <iostream>

#include "theatre/dmxdevice.h"

#include "gui/application.h"

#include "system/writer.h"

int main(int argc, char *argv[]) {
  glight::gui::Application application;
  application.Run(argc, argv);

  return EXIT_SUCCESS;
}
