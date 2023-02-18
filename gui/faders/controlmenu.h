#ifndef GUI_CONTROL_MENU_H_
#define GUI_CONTROL_MENU_H_

#include <gtkmm/checkmenuitem.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/separatormenuitem.h>

namespace glight::gui {

class FaderState;

class ControlMenu : public Gtk::Menu {
 public:
  ControlMenu(const FaderState& state);

  bool DisplayName() const { return _miDisplayName.get_active(); }
  bool DisplayFlashButton() const { return _miDisplayFlashButton.get_active(); }
  bool DisplayCheckButton() const { return _miDisplayCheckButton.get_active(); }
  bool OverlayFadeButtons() const { return _miOverlayFadeButtons.get_active(); }

  Glib::SignalProxy<void> SignalHide() { return signal_hide(); }

  Glib::SignalProxy<void> SignalAssign() { return _miAssign.signal_activate(); }

  Glib::SignalProxy<void> SignalToggleName() {
    return _miDisplayName.signal_activate();
  }

  Glib::SignalProxy<void> SignalToggleFlashButton() {
    return _miDisplayFlashButton.signal_activate();
  }

  Glib::SignalProxy<void> SignalToggleCheckButton() {
    return _miDisplayCheckButton.signal_activate();
  }

  Glib::SignalProxy<void> SignalToggleFadeButtons() {
    return _miOverlayFadeButtons.signal_activate();
  }

 private:
  Gtk::MenuItem _miAssign{"Assign..."};
  Gtk::SeparatorMenuItem _miSeperator1;
  Gtk::CheckMenuItem _miDisplayName{"Display label"};
  Gtk::CheckMenuItem _miDisplayFlashButton{"Display flash button"};
  Gtk::CheckMenuItem _miDisplayCheckButton{"Display check button"};
  Gtk::CheckMenuItem _miOverlayFadeButtons{"Overlay fade buttons"};
};

}  // namespace glight::gui

#endif
