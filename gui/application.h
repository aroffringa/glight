#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <gtkmm/application.h>

namespace glight::gui {

class Application : public Gtk::Application {
 public:
  void Run(int argc, char *argv[]);
};

}  // namespace glight::gui

#endif
