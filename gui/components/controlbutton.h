#ifndef GLIGHT_GUI_COMPONENTS_CONTROL_BUTTON_H_
#define GLIGHT_GUI_COMPONENTS_CONTROL_BUTTON_H_

#include <sigc++/signal.h>

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>

namespace glight::gui {

/**
 * A control button is like a normal Gtk::Button, but adds events
 * for press and release events (which in Gtkmm 4 require an extra
 * box around a Gtk::Button).
 */
class ControlButton : public Gtk::Box {
 public:
  ControlButton(const std::string& label = "");

  sigc::signal<void(int)>& SignalPress() { return signal_press_; }
  sigc::signal<void(int)>& SignalRelease() { return signal_release_; }

  void SetIconName(const char* name) { button_.set_icon_name(name); }

  /**
   * A value of zero (the default) causes any button to send the press and
   * release signals. Otherwise, a value of 1 is left button, 3 is right button.
   */
  void SetSignalButton(int button) { signal_button_ = button; }

 private:
  Gtk::Button button_;
  Gtk::Label label_;
  int signal_button_ = 0;
  sigc::signal<void(int)> signal_press_;
  sigc::signal<void(int)> signal_release_;
};

}  // namespace glight::gui

#endif
