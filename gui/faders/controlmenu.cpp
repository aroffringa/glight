#include "controlmenu.h"

#include "gui/menufunctions.h"
#include "uistate/faderstate.h"

namespace glight::gui {

ControlMenu::ControlMenu(const uistate::FaderState& state)
    : actions_(Gio::SimpleActionGroup::create()) {
  const std::array<Glib::ustring, 11> values{"Off", "10%", "20%", "30%", "40%", "50%",
                                             "60%", "70%", "80%", "90%", "Full"};

  auto menu = Gio::Menu::create();

  auto set_a_to_menu = Gio::Menu::create();
  auto fade_a_to_menu = Gio::Menu::create();
  auto set_b_to_menu = Gio::Menu::create();

  for (size_t i = 0; i != values.size(); ++i) {
    const size_t value = values.size() - i - 1;
    Add(set_a_to_menu, values[value], "set_a_to_" + std::to_string(value), [this, value]() {
      signal_set_to_(theatre::ControlValue::FromRatio(static_cast<double>(value) / 10.0), true);
    });
    Add(fade_a_to_menu, values[value], "fade_a_to_" + std::to_string(value), [this, value]() {
      signal_fade_to_(theatre::ControlValue::FromRatio(static_cast<double>(value) / 10.0));
    });
    Add(set_b_to_menu, values[value], "set_b_to_" + std::to_string(value), [this, value]() {
      signal_set_to_(theatre::ControlValue::FromRatio(static_cast<double>(value) / 10.0), false);
    });
  }

  menu->append_submenu("Set A to", set_a_to_menu);
  menu->append_submenu("Fade A to", fade_a_to_menu);
  menu->append_submenu("Set B to", set_b_to_menu);

  assign_section_ = Gio::Menu::create();
  Add(assign_section_, "Assign...", "assign_fader", signal_assign_);
  Add(assign_section_, "Unassign", "unassign_fader", signal_unassign_);

  menu->append_section(assign_section_);

  auto rest_section = Gio::Menu::create();

  display_flash_button_ = Toggle(rest_section, "Flash button", "toggle_display_flash",
                                 state.DisplayFlashButton(), signal_toggle_flash_button_);
  display_check_button_ = Toggle(rest_section, "Check button", "toggle_display_check",
                                 state.DisplayCheckButton(), signal_toggle_check_button_);
  overlay_fade_buttons_ = Toggle(rest_section, "Overlay fade buttons", "toggle_overlay_fade",
                                 state.OverlayFadeButtons(), signal_toggle_fade_buttons_);

  menu->append_section(rest_section);
  set_menu_model(menu);
}

std::shared_ptr<Gio::SimpleAction> ControlMenu::Add(std::shared_ptr<Gio::Menu>& menu,
                                                    const Glib::ustring& label,
                                                    const Glib::ustring& action_name,
                                                    const sigc::slot<void()>& slot) {
  return AddMenuItem(*actions_, menu, label, action_name, slot, "control.");
};

std::shared_ptr<Gio::SimpleAction> ControlMenu::Toggle(std::shared_ptr<Gio::Menu>& menu,
                                                       const Glib::ustring& label,
                                                       const Glib::ustring& action_name,
                                                       bool initial_value,
                                                       const sigc::slot<void(bool)>& slot) {
  return AddToggleMenuItem(*actions_, menu, label, action_name, initial_value, slot, "control.");
};

}  // namespace glight::gui
