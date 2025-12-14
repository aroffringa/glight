#ifndef GUI_CONTROL_MENU_H_
#define GUI_CONTROL_MENU_H_

#include <vector>

#include <sigc++/signal.h>

#include <giomm/actionmap.h>
#include <giomm/menu.h>
#include <giomm/simpleaction.h>
#include <giomm/simpleactiongroup.h>

#include <gtkmm/popovermenu.h>

namespace glight::uistate {
class FaderState;
}

namespace glight::gui {

class ControlMenu : public Gtk::PopoverMenu {
 public:
  ControlMenu(const uistate::FaderState& state);

  bool DisplayName() const { return GetState(display_name_); }
  bool DisplayFlashButton() const { return GetState(display_flash_button_); }
  bool DisplayCheckButton() const { return GetState(display_check_button_); }
  bool OverlayFadeButtons() const { return GetState(overlay_fade_buttons_); }

  Glib::SignalProxy<void()> SignalHide() { return signal_hide(); }

  sigc::signal<void()>& SignalAssign() { return signal_assign_; }
  sigc::signal<void()>& SignalUnassign() { return signal_unassign_; }

  sigc::signal<void(bool)>& SignalToggleName() { return signal_toggle_name_; }

  sigc::signal<void(bool)>& SignalToggleFlashButton() {
    return signal_toggle_flash_button_;
  }

  sigc::signal<void(bool)>& SignalToggleCheckButton() {
    return signal_toggle_check_button_;
  }

  sigc::signal<void(bool)>& SignalToggleFadeButtons() {
    return signal_toggle_fade_buttons_;
  }

  template <typename Function>
  void AddExtraItem(const std::string& label, Function function) {
    assign_section_->append("Assign...", "assign");
    actions_->add_action("assign", function);
  }

  const std::shared_ptr<Gio::SimpleActionGroup>& GetActionGroup() const {
    return actions_;
  }

 private:
  bool GetState(const std::shared_ptr<Gio::SimpleAction>& action) const {
    bool value;
    action->get_state(value);
    return value;
  }
  void SetState(const std::shared_ptr<Gio::SimpleAction>& action, bool state) {
    action->set_state(Glib::Variant<bool>::create(state));
  }
  std::shared_ptr<Gio::SimpleAction> Add(std::shared_ptr<Gio::Menu>& menu,
                                         const Glib::ustring& label,
                                         const Glib::ustring& action_name,
                                         const sigc::slot<void()>& slot);

  std::shared_ptr<Gio::SimpleAction> Toggle(std::shared_ptr<Gio::Menu>& menu,
                                            const Glib::ustring& label,
                                            const Glib::ustring& action_name,
                                            bool initial_value,
                                            const sigc::slot<void(bool)>& slot);

  std::shared_ptr<Gio::SimpleActionGroup> actions_;
  std::shared_ptr<Gio::Menu> assign_section_;

  sigc::signal<void()> signal_assign_;
  sigc::signal<void()> signal_unassign_;
  sigc::signal<void(bool)> signal_toggle_name_;
  sigc::signal<void(bool)> signal_toggle_flash_button_;
  sigc::signal<void(bool)> signal_toggle_check_button_;
  sigc::signal<void(bool)> signal_toggle_fade_buttons_;

  std::shared_ptr<Gio::SimpleAction> display_name_;
  std::shared_ptr<Gio::SimpleAction> display_flash_button_;
  std::shared_ptr<Gio::SimpleAction> display_check_button_;
  std::shared_ptr<Gio::SimpleAction> overlay_fade_buttons_;
};

}  // namespace glight::gui

#endif
