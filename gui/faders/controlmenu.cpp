#include "controlmenu.h"

#include "../state/faderstate.h"

namespace glight::gui {

ControlMenu::ControlMenu(const FaderState& state) {
  auto menu = Gio::Menu::create();
  assign_section_ = Gio::Menu::create();
  actions_ = Gio::SimpleActionGroup::create();

  assign_section_->append("Assign...", "assign");
  actions_->add_action("assign", signal_assign_);

  assign_section_->append("Unassign", "unassign");
  actions_->add_action("unassign", signal_unassign_);

  menu->append_section(assign_section_);

  auto rest_section = Gio::Menu::create();

  rest_section->append("Display label", "toggle_display_name");
  std::shared_ptr<Gio::SimpleAction> display_name = Gio::SimpleAction::create(
      "toggle_display_name", Glib::Variant<bool>::create(state.DisplayName()));
  actions_->add_action(display_name);

  rest_section->append("Display label", "toggle_display_flash");
  std::shared_ptr<Gio::SimpleAction> display_flash = Gio::SimpleAction::create(
      "toggle_display_flash",
      Glib::Variant<bool>::create(state.DisplayFlashButton()));
  actions_->add_action(display_flash);

  rest_section->append("Display label", "toggle_display_check");
  std::shared_ptr<Gio::SimpleAction> display_check = Gio::SimpleAction::create(
      "toggle_display_check",
      Glib::Variant<bool>::create(state.DisplayCheckButton()));
  actions_->add_action(display_check);

  rest_section->append("Display label", "toggle_overlay_fade");
  std::shared_ptr<Gio::SimpleAction> display_fade = Gio::SimpleAction::create(
      "toggle_overlay_fade",
      Glib::Variant<bool>::create(state.OverlayFadeButtons()));
  actions_->add_action(display_fade);

  menu->append_section(rest_section);
  set_menu_model(menu);
}

}  // namespace glight::gui
