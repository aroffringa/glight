#include "controlbutton.h"

#include <gtkmm/gestureclick.h>

namespace glight::gui {

ControlButton::ControlButton(const std::string& label) : label_(label) {
  button_.set_label(label);
  append(button_);
  // The box is to capture the click signals. Connecting the
  // event controller directly to the button doesn't work in gtkmm 4.
  auto flash_gesture = Gtk::GestureClick::create();
  flash_gesture->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
  flash_gesture->set_button(0);
  flash_gesture->signal_pressed().connect(
      [this, g = flash_gesture.get()](int, double, double) {
        const int button = g->get_current_button();
        if (signal_button_ == 0 || signal_button_ == button)
          signal_press_(button);
      });
  flash_gesture->signal_released().connect(
      [this, g = flash_gesture.get()](int, double, double) {
        const int button = g->get_current_button();
        if (signal_button_ == 0 || signal_button_ == button)
          signal_release_(button);
      });
  add_controller(flash_gesture);
}

}  // namespace glight::gui
