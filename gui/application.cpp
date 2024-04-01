#include "application.h"

#include <iostream>

#include "gui/mainwindow/mainwindow.h"

namespace glight::gui {

void Application::Run(int argc, char *argv[]) {
  MainWindow window;
  if (argc > 1) {
    window.OpenFile(argv[1]);
  }
  run(window, 1, argv);
}

}  // namespace glight::gui
