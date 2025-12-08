#include "mainmenu.h"

#include <giomm/actionmap.h>
#include <giomm/simpleactiongroup.h>
#include <giomm/menu.h>

#include "gui/state/fadersetstate.h"
#include "gui/menufunctions.h"

#include <iostream>  // DEBUG

namespace glight::gui {

MainMenu::MainMenu(Gio::ActionMap& actions) {
  const auto Add = [&actions](std::shared_ptr<Gio::Menu>& menu,
                              const Glib::ustring& label,
                              const Glib::ustring& action_name,
                              const sigc::slot<void()>& slot) {
    return AddMenuItem(actions, menu, label, action_name, slot);
  };

  const auto Toggle =
      [&actions](std::shared_ptr<Gio::Menu>& menu, const Glib::ustring& label,
                 const Glib::ustring& action_name, bool initial_value,
                 const sigc::slot<void(bool)>& slot) {
        return AddToggleMenuItem(actions, menu, label, action_name,
                                 initial_value, slot);
      };

  // File menu
  auto file_menu = Gio::Menu::create();

  auto file_section = Gio::Menu::create();
  Add(file_section, "New", "file_new", New);
  Add(file_section, "_Open...", "file_open", Open);
  Add(file_section, "Save _as...", "file_save_as", Save);
  Add(file_section, "_Import fixtures...", "import_fixtures", Import);
  file_menu->append_section(file_section);

  auto settings_section = Gio::Menu::create();
  Add(file_menu, "_Settings...", "file_settings", Settings);
  file_menu->append_section(settings_section);

  auto quit_section = Gio::Menu::create();
  Add(file_menu, "_Quit", "file_quit", Quit);
  file_menu->append_section(quit_section);

  auto design_menu = Gio::Menu::create();
  auto design_section = Gio::Menu::create();
  layout_locked_ =
      Toggle(design_section, "Lock layout", "design_lock", false, LockLayout);
  auto blackout_action =
      Add(design_section, "Black-out", "black_out", BlackOut);
  blackout_action->set_enabled(false);
  Toggle(design_section, "Protect black-out", "protect_lock", true,
         [blackout_action](bool new_value) {
           blackout_action->set_enabled(!new_value);
         });

  design_menu->append_section(design_section);

  auto add_section = Gio::Menu::create();
  auto preset_menu = Gio::Menu::create();
  Add(preset_menu, "Empty", "add_empty_preset", AddEmptyPreset);
  Add(preset_menu, "From current", "add_current_preset", AddCurrentPreset);
  add_section->append_submenu("Add preset", preset_menu);

  Add(add_section, "Add chase", "add_chase", AddChase);
  Add(add_section, "Add sequence", "add_sequence", AddTimeSequence);

  auto effect_menu = Gio::Menu::create();
  std::vector<theatre::EffectType> effect_types = theatre::GetEffectTypes();
  for (theatre::EffectType t : effect_types) {
    Add(effect_menu, EffectTypeToName(t),
        "add_effect_" + std::to_string(static_cast<int>(t)),
        [&, t]() { AddEffect(t); });
  }
  add_section->append_submenu("Add effect", effect_menu);
  Add(add_section, "Add folder", "add_folder", AddFolder);
  delete_object_ = Add(add_section, "Delete", "delete_object", DeleteObject);
  Add(add_section, "Design wizard...", "design_wizard", DesignWizard);
  design_menu->append_section(add_section);

  auto dimensions_section = Gio::Menu::create();
  Add(add_section, "Theatre dimensions...", "theatre_dimensions",
      TheatreDimensions);
  design_menu->append_section(dimensions_section);

  auto view_menu = Gio::Menu::create();
  show_fixtures_ =
      Toggle(view_menu, "Show fixtures", "show_fixtures", true, ShowFixtures);
  show_beams_ = Toggle(view_menu, "Show beams", "show_beams", true, ShowBeams);
  show_projections_ = Toggle(view_menu, "Show projections", "show_projections",
                             true, ShowProjections);
  show_stage_borders_ = Toggle(view_menu, "Show theatre walls",
                               "show_stage_borders", true, ShowStageBorders);
  full_screen_ =
      Toggle(view_menu, "Full screen", "full_screen", false, FullScreen);

  auto window_menu = Gio::Menu::create();
  side_bar_ = Toggle(window_menu, "Side bar", "side_bar", true, SideBar);
  power_monitor_ = Toggle(window_menu, "Power monitor", "power_monitor", false,
                          PowerMonitor);
  fixture_list_ =
      Toggle(window_menu, "Fixtures", "fixture_list", false, FixtureList);
  fixture_types_ = Toggle(window_menu, "Fixture types", "fixture_types", false,
                          FixtureTypes);
  scene_window_ = Toggle(window_menu, "Scene", "scene", false, SceneWindow);

  auto fader_window_menu = Gio::Menu::create();
  auto fader_window_section = Gio::Menu::create();
  Add(fader_window_section, "New", "new_fader_window",
      [&]() { NewFaderWindow(); });
  fader_window_menu->append_section(fader_window_section);
  window_menu->append_submenu("Fader windows", fader_window_menu);

  auto top_level_menu = Gio::Menu::create();
  top_level_menu->append_submenu("_File", file_menu);
  top_level_menu->append_submenu("_Design", design_menu);
  top_level_menu->append_submenu("_View", view_menu);
  top_level_menu->append_submenu("_Window", window_menu);
  set_menu_model(top_level_menu);
}

void MainMenu::SetFaderList(
    const std::vector<std::unique_ptr<FaderSetState>>& faders) {
  /*_miFaderWindows.clear();

  for (const std::unique_ptr<FaderSetState>& state : faders) {
    Gtk::CheckMenuItem& item = _miFaderWindows.emplace_back(state->name);
    item.set_active(state->isActive);
    item.signal_toggled().connect([&]() { FaderWindow(*state); });
    item.show();
    _menuFaderWindows.append(item);
  }*/
}

}  // namespace glight::gui
