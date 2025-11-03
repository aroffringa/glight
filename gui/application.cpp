#include "application.h"

#include <iostream>

#include <alsa/asoundlib.h>

#include "gui/mainwindow/mainwindow.h"

namespace glight::gui {

void Application::Run(int argc, char *argv[]) {
  MainWindow window;
  if (argc > 1) {
    window.OpenFile(argv[1]);
  }
  signal_activate().connect([&window, this]() {
    add_window(window);
    window.present();
  });
  run();
  snd_config_update_free_global();
}

}  // namespace glight::gui
