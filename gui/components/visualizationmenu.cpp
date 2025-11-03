#include "visualizationmenu.h"

#include "theatre/color.h"
#include "theatre/management.h"

namespace glight::gui {

using theatre::Color;

VisualizationMenu::VisualizationMenu(Gio::ActionMap& map) {
  auto menu = Gio::Menu::create();

  auto set_menu = Gio::Menu::create();
  set_menu->append("Full on", "set_full_on");
  set_menu->append("Off", "set_off");
  set_menu->append("Set color...", "set_color");
  set_menu->append("Track", "track");
  set_menu->append("Pan only", "pan_track");

  set_full_on_ = map.add_action("set_full_on", SignalSetFullOn);
  set_off_ = map.add_action("set_off", SignalSetOff);
  set_color_ = map.add_action("set_color", SignalSetColor);
  set_track_ = map.add_action("track", SignalTrack);
  set_pan_track_ = map.add_action("pan_track", SignalTrackPan);

  menu->append_submenu("Set", set_menu);

  auto symbol_menu = Gio::Menu::create();
  const std::vector<theatre::FixtureSymbol::Symbol> symbols(
      theatre::FixtureSymbol::List());
  for (theatre::FixtureSymbol::Symbol symbol : symbols) {
    symbol_menu->append(theatre::FixtureSymbol(symbol).Name(),
                        "symbol_" + theatre::FixtureSymbol(symbol).Name());
    std::shared_ptr<Gio::SimpleAction> action =
        map.add_action("symbol_" + theatre::FixtureSymbol(symbol).Name(),
                       [&, symbol]() { SignalSelectSymbol(symbol); });
    symbols_.emplace_back(std::move(action));
  }
  menu->append_submenu("Symbol", symbol_menu);

  // TODO move this to main menu instead of context menu
  auto dry_mode_style_menu = Gio::Menu::create();
  dry_mode_style_menu->append("Primary", "mode_primary");
  dry_mode_style_menu->append("Secondary", "mode_secondary");
  dry_mode_style_menu->append("Vertical", "mode_vertical");
  dry_mode_style_menu->append("Horizontal", "mode_horizontal");
  dry_mode_style_menu->append("Shadow", "mode_shadow");

  map.add_action("mode_primary", [&]() {
    dry_mode_style_ = DryModeStyle::Primary;
    SignalDryStyleChange();
  });
  map.add_action("mode_secondary", [&]() {
    dry_mode_style_ = DryModeStyle::Secondary;
    SignalDryStyleChange();
  });
  map.add_action("mode_vertical", [&]() {
    dry_mode_style_ = DryModeStyle::Vertical;
    SignalDryStyleChange();
  });
  map.add_action("mode_horizontal", [&]() {
    dry_mode_style_ = DryModeStyle::Horizontal;
    SignalDryStyleChange();
  });
  map.add_action("mode_shadow", [&]() {
    dry_mode_style_ = DryModeStyle::Shadow;
    SignalDryStyleChange();
  });

  menu->append_submenu("Dry mode style", dry_mode_style_menu);

  auto position_section = Gio::Menu::create();
  position_section->append("Align horizontally", "align_horizontally");
  position_section->append("Align vertically", "align_vertically");
  position_section->append("Distribute evenly", "distribute_evenly");
  menu->append_section(position_section);

  align_horizontally_ =
      map.add_action("align_horizontally", SignalAlignHorizontally);
  align_vertically_ = map.add_action("align_vertically", SignalAlignVertically);
  distribute_evenly_ =
      map.add_action("distribute_evenly", SignalDistributeEvenly);

  auto edit_section = Gio::Menu::create();
  edit_section->append("Add fixture...", "add_fixture");
  edit_section->append("Add preset", "add_preset");
  edit_section->append("Remove", "remove_fixtures");
  edit_section->append("Group...", "group_fixtures");
  edit_section->append("Design...", "design");
  menu->append_section(edit_section);

  add_fixture_ = map.add_action("add_fixture", SignalAddFixtures);
  add_preset_ = map.add_action("add_preset", SignalAddPreset);
  remove_fixtures_ = map.add_action("remove_fixtures", SignalRemoveFixtures);
  group_fixtures_ = map.add_action("group_fixtures", SignalGroupFixtures);
  design_ = map.add_action("design", SignalDesignFixtures);

  auto extra_section = Gio::Menu::create();
  extra_section->append("Properties", "properties");
  extra_section->append("Save image...", "save_image");
  menu->append_section(extra_section);

  properties_ = map.add_action("properties", SignalAddFixtures);
  save_image_ = map.add_action("save_image", SignalAddPreset);
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
