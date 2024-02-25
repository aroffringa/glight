#ifndef GUI_CONTROL_MENU_H_
#define GUI_CONTROL_MENU_H_

#include <vector>

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
  Glib::SignalProxy<void> SignalUnassign() {
    return _miUnassign.signal_activate();
  }

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

  template <typename Function>
  void AddExtraItem(const std::string& label, Function function) {
    Gtk::MenuItem& item = extra_items_.emplace_back(label);
    item.signal_activate().connect(function);
    // Add the item before the first separator (after assign & unasssign)
    insert(item, 1 + extra_items_.size());
    item.show();
  }

 private:
  Gtk::MenuItem _miAssign{"Assign..."};
  Gtk::MenuItem _miUnassign{"Unassign"};
  std::vector<Gtk::MenuItem> extra_items_;
  Gtk::SeparatorMenuItem _miSeperator1;
  Gtk::CheckMenuItem _miDisplayName{"Display label"};
  Gtk::CheckMenuItem _miDisplayFlashButton{"Display flash button"};
  Gtk::CheckMenuItem _miDisplayCheckButton{"Display check button"};
  Gtk::CheckMenuItem _miOverlayFadeButtons{"Overlay fade buttons"};
};

}  // namespace glight::gui

#endif
