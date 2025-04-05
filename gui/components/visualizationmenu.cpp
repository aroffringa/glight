#include "visualizationmenu.h"

#include "theatre/color.h"
#include "theatre/management.h"

namespace glight::gui {

using theatre::Color;

VisualizationMenu::VisualizationMenu() {
  std::vector<theatre::FixtureSymbol::Symbol> symbols(
      theatre::FixtureSymbol::List());

  set_menu_.add(mi_set_full_on_);
  mi_set_full_on_.signal_activate().connect(SignalSetFullOn);

  set_menu_.add(mi_set_off_);
  mi_set_off_.signal_activate().connect(SignalSetOff);

  set_menu_.add(mi_set_color_);
  mi_set_color_.signal_activate().connect(SignalSetColor);

  set_menu_.add(mi_track_);
  mi_track_.signal_activate().connect(SignalTrack);

  set_menu_.add(mi_track_pan_);
  mi_track_pan_.signal_activate().connect(SignalTrackPan);

  mi_set_menu_.set_submenu(set_menu_);
  add(mi_set_menu_);

  _miSymbols.reserve(symbols.size());
  for (theatre::FixtureSymbol::Symbol symbol : symbols) {
    _miSymbols.emplace_back(theatre::FixtureSymbol(symbol).Name());
    _miSymbols.back().signal_activate().connect(
        [&, symbol]() { SignalSelectSymbol(symbol); });
    _symbolMenu.add(_miSymbols.back());
  }

  _miSymbolMenu.set_submenu(_symbolMenu);
  add(_miSymbolMenu);

  Gtk::RadioMenuItem::Group dryModeStyleGroup;
  _miDMSPrimary.set_group(dryModeStyleGroup);
  _miDMSPrimary.set_active(true);
  _miDMSPrimary.signal_activate().connect(SignalDryStyleChange);
  _dryModeStyleMenu.add(_miDMSPrimary);
  _miDMSSecondary.set_group(dryModeStyleGroup);
  _miDMSSecondary.signal_activate().connect(SignalDryStyleChange);
  _dryModeStyleMenu.add(_miDMSSecondary);
  _miDMSHorizontal.set_group(dryModeStyleGroup);
  _miDMSHorizontal.signal_activate().connect(SignalDryStyleChange);
  _dryModeStyleMenu.add(_miDMSHorizontal);
  _miDMSVertical.set_group(dryModeStyleGroup);
  _miDMSVertical.signal_activate().connect(SignalDryStyleChange);
  _dryModeStyleMenu.add(_miDMSVertical);
  _miDMSShadow.set_group(dryModeStyleGroup);
  _miDMSShadow.signal_activate().connect(SignalDryStyleChange);
  _dryModeStyleMenu.add(_miDMSShadow);

  _miDryModeStyle.set_submenu(_dryModeStyleMenu);
  add(_miDryModeStyle);

  _miAlignHorizontally.signal_activate().connect(
      [&] { SignalAlignHorizontally(); });
  add(_miAlignHorizontally);

  _miAlignVertically.signal_activate().connect(SignalAlignVertically);
  add(_miAlignVertically);

  _miDistributeEvenly.signal_activate().connect(SignalDistributeEvenly);
  add(_miDistributeEvenly);

  add(_miSeparator1);

  _miAdd.signal_activate().connect(SignalAddFixtures);
  add(_miAdd);

  _miAddPreset.signal_activate().connect(SignalAddPreset);
  add(_miAddPreset);

  _miRemove.signal_activate().connect(SignalRemoveFixtures);
  add(_miRemove);

  _miGroup.signal_activate().connect(SignalGroupFixtures);
  add(_miGroup);

  _miDesign.signal_activate().connect(SignalDesignFixtures);
  add(_miDesign);

  add(_miSeparator2);

  _miProperties.signal_activate().connect(SignalFixtureProperties);
  add(_miProperties);

  _miSaveImage.signal_activate().connect(SignalSaveImage);
  add(_miSaveImage);

  show_all_children();
}

DryModeStyle VisualizationMenu::GetDryModeStyle() const {
  if (_miDMSSecondary.get_active())
    return DryModeStyle::Secondary;
  else if (_miDMSHorizontal.get_active())
    return DryModeStyle::Horizontal;
  else if (_miDMSVertical.get_active())
    return DryModeStyle::Vertical;
  else if (_miDMSShadow.get_active())
    return DryModeStyle::Shadow;
  else
    return DryModeStyle::Primary;
}

void VisualizationMenu::SetSensitivity(bool is_layout_locked,
                                       size_t n_selected) {
  const bool has_selection = n_selected != 0;
  const bool selection_enabled = !is_layout_locked && has_selection;
  const bool dual_enabled = !is_layout_locked && n_selected >= 2;
  mi_set_menu_.set_sensitive(has_selection);
  _miAlignHorizontally.set_sensitive(dual_enabled);
  _miAlignVertically.set_sensitive(dual_enabled);
  _miDistributeEvenly.set_sensitive(dual_enabled);
  _miAdd.set_sensitive(!is_layout_locked);
  _miAddPreset.set_sensitive(!is_layout_locked);
  _miRemove.set_sensitive(selection_enabled);
  _miSymbolMenu.set_sensitive(selection_enabled);
  _miProperties.set_sensitive(selection_enabled);
}

}  // namespace glight::gui
