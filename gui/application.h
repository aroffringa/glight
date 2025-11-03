#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <gtkmm/application.h>

namespace glight::gui {

class Application : public Gtk::Application {
 public:
  Application()
      : Gtk::Application("org.glight", Gio::Application::Flags::HANDLES_OPEN) {}
  void Run(int argc, char *argv[]);
};

}  // namespace glight::gui

#endif
