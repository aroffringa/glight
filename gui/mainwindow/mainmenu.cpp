#include "mainmenu.h"

#include <giomm/actionmap.h>
#include <giomm/simpleactiongroup.h>
#include <giomm/menu.h>

#include "gui/state/fadersetstate.h"

namespace glight::gui {

MainMenu::MainMenu() {
  std::shared_ptr<Gio::SimpleActionGroup> actions =
      Gio::SimpleActionGroup::create();

  const auto Add = [&actions](std::shared_ptr<Gio::Menu>& menu,
                              const Glib::ustring& label,
                              const Glib::ustring& action_name,
                              const sigc::slot<void()>& slot) {
    menu->append(label, action_name);
    return actions->add_action(action_name, slot);
  };

  const auto Toggle = [&actions](std::shared_ptr<Gio::Menu>& menu,
                                 const Glib::ustring& label,
                                 const Glib::ustring& action_name,
                                 const sigc::slot<void()>& slot) {
    auto action_toggle = Gio::SimpleAction::create_bool(action_name, false);
    menu->append(label, action_name);
    actions->add_action(action_toggle);
    action_toggle->signal_activate().connect(
        [slot, action_toggle](const Glib::VariantBase&) {
          bool active = false;
          action_toggle->get_state(active);
          active = !active;
          action_toggle->change_state(Glib::Variant<bool>::create(active));
          slot();
        });
    return action_toggle;
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
      Toggle(design_section, "Lock layout", "design_lock", LockLayout);
  auto blackout_action =
      Add(design_section, "Black-out", "black_out", BlackOut);
  Add(design_section, "Protect black-out", "protect_lock",
      [this, blackout_action]() {
        bool protect;
        layout_locked_->get_state(protect);
        blackout_action->set_enabled(!protect);
      });
  design_menu->append_section(design_section);

  auto add_section = Gio::Menu::create();
  auto preset_menu = Gio::Menu::create();
  Add(preset_menu, "Empty", "add_empty_preset", AddEmptyPreset);
  Add(preset_menu, "From current", "add_current_preset", AddCurrentPreset);
  add_section->append_submenu("Add preset", preset_menu);

  Add(add_section, "Add chase", "design_lock", AddChase);
  Add(add_section, "Add sequence", "design_lock", AddTimeSequence);

  auto effect_menu = Gio::Menu::create();
  std::vector<theatre::EffectType> effect_types = theatre::GetEffectTypes();
  for (theatre::EffectType t : effect_types) {
    Add(effect_menu, EffectTypeToName(t), "add_effect_" + EffectTypeToName(t),
        [&, t]() { AddEffect(t); });
  }
  add_section->append_submenu("Add effect", effect_menu);
  Add(add_section, "Add folder", "add_folder", AddFolder);
  Add(add_section, "Delete", "design_lock", DeleteObject);
  Add(add_section, "Design wizard...", "", DesignWizard);
  design_menu->append_section(add_section);

  auto dimensions_section = Gio::Menu::create();
  Add(add_section, "Theatre dimensions...", "", TheatreDimensions);
  design_menu->append_section(dimensions_section);

  auto view_menu = Gio::Menu::create();
  show_fixtures_ =
      Toggle(view_menu, "Show fixtures", "show_fixtures", ShowFixtures);
  show_beams_ = Toggle(view_menu, "Show beams", "show_beams", ShowBeams);
  show_projections_ = Toggle(view_menu, "Show projections", "show_projections",
                             ShowProjections);
  show_stage_borders_ = Toggle(view_menu, "Show theatre walls",
                               "show_stage_borders", ShowStageBorders);
  full_screen_ = Toggle(view_menu, "Full screen", "full_screen", FullScreen);

  auto window_menu = Gio::Menu::create();
  side_bar_ = Toggle(window_menu, "Side bar", "side_bar", SideBar);
  power_monitor_ =
      Toggle(window_menu, "Power monitor", "power_monitor", PowerMonitor);
  fixture_list_ = Toggle(window_menu, "Fixtures", "fixture_list", FixtureList);
  fixture_types_ =
      Toggle(window_menu, "Fixture types", "fixture_types", FixtureTypes);
  scene_window_ = Add(window_menu, "Scene", "scene",
                      [&]() { SceneWindow(GetState(scene_window_)); });

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
