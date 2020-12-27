#include <cstdlib>
#include <iostream>

#include "theatre/dmxdevice.h"

#include "gui/application.h"

#include "system/writer.h"

using namespace std;

int main(int argc, char *argv[]) {
  Writer::CheckXmlVersion();

  Application application;
  application.Run(argc, argv);

  return EXIT_SUCCESS;
}
