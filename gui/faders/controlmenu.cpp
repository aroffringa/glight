#include "controlmenu.h"

#include "gui/menufunctions.h"
#include "gui/state/faderstate.h"

namespace glight::gui {

ControlMenu::ControlMenu(const FaderState& state)
    : actions_(Gio::SimpleActionGroup::create()) {
  auto menu = Gio::Menu::create();
  assign_section_ = Gio::Menu::create();

  Add(assign_section_, "Assign...", "assign", signal_assign_);
  Add(assign_section_, "Unassign", "unassign", signal_unassign_);

  menu->append_section(assign_section_);

  auto rest_section = Gio::Menu::create();

  display_name_ = Toggle(rest_section, "Label", "toggle_display_name",
                         state.DisplayName(), signal_toggle_name_);
  display_flash_button_ =
      Toggle(rest_section, "Flash button", "toggle_display_flash",
             state.DisplayFlashButton(), signal_toggle_flash_button_);
  display_check_button_ =
      Toggle(rest_section, "Check button", "toggle_display_check",
             state.DisplayCheckButton(), signal_toggle_check_button_);
  overlay_fade_buttons_ =
      Toggle(rest_section, "Overlay fade buttons", "toggle_overlay_fade",
             state.OverlayFadeButtons(), signal_toggle_fade_buttons_);

  menu->append_section(rest_section);
  set_menu_model(menu);
}

std::shared_ptr<Gio::SimpleAction> ControlMenu::Add(
    std::shared_ptr<Gio::Menu>& menu, const Glib::ustring& label,
    const Glib::ustring& action_name, const sigc::slot<void()>& slot) {
  return AddMenuItem(*actions_, menu, label, action_name, slot);
};

std::shared_ptr<Gio::SimpleAction> ControlMenu::Toggle(
    std::shared_ptr<Gio::Menu>& menu, const Glib::ustring& label,
    const Glib::ustring& action_name, bool initial_value,
    const sigc::slot<void(bool)>& slot) {
  return AddToggleMenuItem(*actions_, menu, label, action_name, initial_value,
                           slot);
};

}  // namespace glight::gui
