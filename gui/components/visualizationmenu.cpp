#include "visualizationmenu.h"

#include "theatre/color.h"
#include "theatre/management.h"

#include "gui/menufunctions.h"

namespace glight::gui {

using theatre::Color;

VisualizationMenu::VisualizationMenu(Gio::ActionMap& actions) {
  const auto Add = [&actions](std::shared_ptr<Gio::Menu>& menu,
                              const Glib::ustring& label,
                              const Glib::ustring& action_name,
                              const sigc::slot<void()>& slot) {
    return AddMenuItem(actions, menu, label, action_name, slot);
  };

  auto menu = Gio::Menu::create();

  auto set_menu = Gio::Menu::create();
  set_full_on_ = Add(set_menu, "Full on", "set_full_on", SignalSetFullOn);
  set_off_ = Add(set_menu, "Off", "set_off", SignalSetOff);
  set_color_ = Add(set_menu, "Set color...", "set_color", SignalSetColor);
  set_track_ = Add(set_menu, "Track", "track", SignalTrack);
  set_pan_track_ = Add(set_menu, "Pan only", "pan_track", SignalTrackPan);

  menu->append_submenu("Set", set_menu);

  auto symbol_menu = Gio::Menu::create();
  const std::vector<theatre::FixtureSymbol::Symbol> symbols(
      theatre::FixtureSymbol::List());
  for (theatre::FixtureSymbol::Symbol symbol : symbols) {
    auto action = Add(symbol_menu, theatre::FixtureSymbol(symbol).Name(),
                      "symbol_" + theatre::FixtureSymbol(symbol).Name(),
                      [&, symbol]() { SignalSelectSymbol(symbol); });
    symbols_.emplace_back(std::move(action));
  }
  menu->append_submenu("Symbol", symbol_menu);

  // TODO move this to main menu instead of context menu
  auto dry_mode_style_menu = Gio::Menu::create();
  Add(dry_mode_style_menu, "Primary", "mode_primary", [&]() {
    dry_mode_style_ = DryModeStyle::Primary;
    SignalDryStyleChange();
  });
  Add(dry_mode_style_menu, "Secondary", "mode_secondary", [&]() {
    dry_mode_style_ = DryModeStyle::Secondary;
    SignalDryStyleChange();
  });
  Add(dry_mode_style_menu, "Vertical", "mode_vertical", [&]() {
    dry_mode_style_ = DryModeStyle::Vertical;
    SignalDryStyleChange();
  });
  Add(dry_mode_style_menu, "Horizontal", "mode_horizontal", [&]() {
    dry_mode_style_ = DryModeStyle::Horizontal;
    SignalDryStyleChange();
  });
  Add(dry_mode_style_menu, "Shadow", "mode_shadow", [&]() {
    dry_mode_style_ = DryModeStyle::Shadow;
    SignalDryStyleChange();
  });

  menu->append_submenu("Dry mode style", dry_mode_style_menu);

  auto position_section = Gio::Menu::create();
  align_horizontally_ = Add(position_section, "Align horizontally",
                            "align_horizontally", SignalAlignHorizontally);
  align_vertically_ = Add(position_section, "Align vertically",
                          "align_vertically", SignalAlignVertically);
  distribute_evenly_ = Add(position_section, "Distribute evenly",
                           "distribute_evenly", SignalDistributeEvenly);
  menu->append_section(position_section);

  auto edit_section = Gio::Menu::create();
  add_fixture_ =
      Add(edit_section, "Add fixture...", "add_fixture", SignalAddFixtures);
  add_preset_ = Add(edit_section, "Add preset", "add_preset", SignalAddPreset);
  remove_fixtures_ =
      Add(edit_section, "Remove", "remove_fixtures", SignalRemoveFixtures);
  group_fixtures_ =
      Add(edit_section, "Group...", "group_fixtures", SignalGroupFixtures);
  design_ = Add(edit_section, "Design...", "design", SignalDesignFixtures);
  menu->append_section(edit_section);

  auto extra_section = Gio::Menu::create();
  properties_ =
      Add(extra_section, "Properties", "properties", SignalFixtureProperties);
  Add(extra_section, "Save image...", "save_image", SignalSaveImage);
  menu->append_section(extra_section);

  set_menu_model(menu);
}

DryModeStyle VisualizationMenu::GetDryModeStyle() const {
  return dry_mode_style_;
}

void VisualizationMenu::SetSensitivity(bool is_layout_locked,
                                       size_t n_selected) {
  const bool has_selection = n_selected != 0;
  const bool selection_enabled = !is_layout_locked && has_selection;
  const bool dual_enabled = !is_layout_locked && n_selected >= 2;

  set_full_on_->set_enabled(has_selection);
  set_off_->set_enabled(has_selection);
  set_color_->set_enabled(has_selection);
  set_track_->set_enabled(has_selection);
  set_pan_track_->set_enabled(has_selection);

  align_horizontally_->set_enabled(dual_enabled);
  align_vertically_->set_enabled(dual_enabled);
  distribute_evenly_->set_enabled(dual_enabled);

  add_fixture_->set_enabled(!is_layout_locked);
  add_preset_->set_enabled(!is_layout_locked);
  remove_fixtures_->set_enabled(selection_enabled);

  properties_->set_enabled(selection_enabled);

  for (auto item : symbols_) item->set_enabled(selection_enabled);
}

}  // namespace glight::gui
