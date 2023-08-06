#include "application.h"

#include <iostream>

#include <gtkmm/main.h>

#include "windows/showwindow.h"

#include "../theatre/dummydevice.h"
#include "../theatre/oladevice.h"

namespace glight::gui {

void Application::Run(int argc, char *argv[]) {
  std::unique_ptr<theatre::DmxDevice> device;
  bool isOpen = false;
  try {
    device = std::make_unique<theatre::OLADevice>();
    device->Open();
    isOpen = device->IsOpen();
  } catch (std::exception &e) {
    std::cerr << "DMX device threw exception: " << e.what() << '\n';
  }
  if (!isOpen) {
    std::cerr << "DMX device not working, switching to dummy device.\n";
    device = std::make_unique<theatre::DummyDevice>();
  }
  ShowWindow window(std::move(device));
  if (argc > 1) {
    window.OpenFile(argv[1]);
  }
  run(window, argc, argv);
}

}  // namespace glight::gui
