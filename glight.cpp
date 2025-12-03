#include <cstdlib>
#include <iostream>

#include <glibmm/init.h>

#include "gui/application.h"

#include "system/writer.h"

int main(int argc, char *argv[]) {
  Glib::set_init_to_users_preferred_locale(false);
  glight::gui::Application application;
  application.Run(argc, argv);

  return EXIT_SUCCESS;
}
