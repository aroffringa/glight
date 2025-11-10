#ifndef GLIGHT_GUI_MENU_FUNCTIONS_H_
#define GLIGHT_GUI_MENU_FUNCTIONS_H_

#include <giomm/actionmap.h>
#include <giomm/menu.h>
#include <giomm/simpleaction.h>

namespace glight::gui {

inline std::shared_ptr<Gio::SimpleAction> AddMenuItem(
    Gio::ActionMap& actions, std::shared_ptr<Gio::Menu>& menu,
    const Glib::ustring& label, const Glib::ustring& action_name,
    const sigc::slot<void()>& slot) {
  menu->append(label, "win." + action_name);
  return actions.add_action(action_name, slot);
};

inline std::shared_ptr<Gio::SimpleAction> AddToggleMenuItem(
    Gio::ActionMap& actions, std::shared_ptr<Gio::Menu>& menu,
    const Glib::ustring& label, const Glib::ustring& action_name,
    bool initial_value, const sigc::slot<void(bool)>& slot) {
  auto action_toggle =
      Gio::SimpleAction::create_bool(action_name, initial_value);
  menu->append(label, "win." + action_name);
  action_toggle->signal_activate().connect(
      [slot, action_toggle](const Glib::VariantBase&) {
        bool active = false;
        action_toggle->get_state(active);
        active = !active;
        action_toggle->change_state(Glib::Variant<bool>::create(active));
        slot(active);
      });
  actions.add_action(action_toggle);
  return action_toggle;
};

}  // namespace glight::gui

#endif
