#include "mainmenu.h"

#include "gui/state/fadersetstate.h"

namespace glight::gui {

MainMenu::MainMenu() {
  // File menu
  _miNew.signal_activate().connect(New);
  file_menu_.append(_miNew);

  _miOpen.signal_activate().connect(Open);
  file_menu_.append(_miOpen);

  _miSave.signal_activate().connect(Save);
  file_menu_.append(_miSave);

  _miImport.signal_activate().connect(Import);
  file_menu_.append(_miImport);

  file_menu_.append(file_sep1_mi_);

  settings_mi_.signal_activate().connect(Settings);
  file_menu_.append(settings_mi_);

  file_menu_.append(file_sep2_mi_);

  _miQuit.signal_activate().connect(Quit);
  file_menu_.append(_miQuit);

  _miFile.set_submenu(file_menu_);
  append(_miFile);

  // Design menu
  _miLockLayout.signal_activate().connect(LockLayout);
  _menuDesign.append(_miLockLayout);

  _miProtectBlackout.signal_activate().connect([&]() {
    bool protect = _miProtectBlackout.get_active();
    _miBlackOut.set_sensitive(!protect);
  });
  _miProtectBlackout.set_active(true);
  _menuDesign.append(_miProtectBlackout);

  _miBlackOut.signal_activate().connect(BlackOut);
  _miBlackOut.set_sensitive(false);
  _menuDesign.append(_miBlackOut);

  _menuDesign.append(_miDesignSep1);

  _miAddPreset.set_submenu(preset_sub_menu_);
  _menuDesign.append(_miAddPreset);
  _miAddEmptyPreset.signal_activate().connect(AddEmptyPreset);
  preset_sub_menu_.append(_miAddEmptyPreset);
  _miAddCurrentPreset.signal_activate().connect(AddCurrentPreset);
  preset_sub_menu_.append(_miAddCurrentPreset);

  _miAddChase.signal_activate().connect(AddChase);
  _menuDesign.append(_miAddChase);
  _miAddSequence.signal_activate().connect(AddTimeSequence);
  _menuDesign.append(_miAddSequence);
  _miAddEffect.set_submenu(effect_sub_menu_);
  _menuDesign.append(_miAddEffect);
  std::vector<theatre::EffectType> effect_types = theatre::GetEffectTypes();
  for (theatre::EffectType t : effect_types) {
    Gtk::MenuItem& mi = effect_menu_items_.emplace_back(EffectTypeToName(t));
    mi.signal_activate().connect([&, t]() { AddEffect(t); });
    effect_sub_menu_.append(mi);
  }
  _miAddFolder.signal_activate().connect(AddFolder);
  _menuDesign.append(_miAddFolder);
  _miDeleteObject.set_sensitive(false);
  _miDeleteObject.signal_activate().connect(DeleteObject);
  _menuDesign.append(_miDeleteObject);

  _miDesignWizard.signal_activate().connect(DesignWizard);
  _menuDesign.append(_miDesignWizard);
  _menuDesign.append(_miDesignSep2);

  _miTheatreDimensions.signal_activate().connect(TheatreDimensions);
  _menuDesign.append(_miTheatreDimensions);

  _miDesign.set_submenu(_menuDesign);
  append(_miDesign);

  // View menu
  _miShowFixtures.set_active(true);
  _miShowFixtures.signal_activate().connect(ShowFixtures);
  _menuView.append(_miShowFixtures);

  _miShowBeams.set_active(true);
  _miShowBeams.signal_activate().connect(ShowBeams);
  _menuView.append(_miShowBeams);

  _miShowProjections.set_active(true);
  _miShowProjections.signal_activate().connect(ShowProjections);
  _menuView.append(_miShowProjections);

  _miShowStageBorders.set_active(true);
  _miShowStageBorders.signal_activate().connect(ShowStageBorders);
  _menuView.append(_miShowStageBorders);

  _menuView.append(_miViewSeperator);

  _miFullScreen.signal_activate().connect(FullScreen);
  _menuView.append(_miFullScreen);

  _miView.set_submenu(_menuView);
  append(_miView);

  // Window menu
  _miSideBar.set_active(true);
  _miSideBar.signal_activate().connect(SideBar);
  _menuWindow.append(_miSideBar);

  _miPowerMonitor.signal_activate().connect(PowerMonitor);
  _menuWindow.append(_miPowerMonitor);

  _miNewFaderWindow.signal_activate().connect([&]() { NewFaderWindow(); });
  _menuFaderWindows.append(_miNewFaderWindow);
  _menuFaderWindows.append(_miFaderWindowSeperator);

  _miFaderWindowMenu.set_submenu(_menuFaderWindows);
  _menuWindow.append(_miFaderWindowMenu);

  _miFixtureListWindow.set_active(false);
  _miFixtureListWindow.signal_activate().connect(FixtureList);
  _menuWindow.append(_miFixtureListWindow);

  _miFixtureTypesWindow.set_active(false);
  _miFixtureTypesWindow.signal_activate().connect(FixtureTypes);
  _menuWindow.append(_miFixtureTypesWindow);

  _miSceneWindow.set_active(false);
  _miSceneWindow.signal_activate().connect(
      [&]() { SceneWindow(_miSceneWindow.get_active()); });
  _menuWindow.append(_miSceneWindow);

  _miWindow.set_submenu(_menuWindow);
  append(_miWindow);
}

void MainMenu::SetFaderList(
    const std::vector<std::unique_ptr<FaderSetState>>& faders) {
  _miFaderWindows.clear();

  for (const std::unique_ptr<FaderSetState>& state : faders) {
    Gtk::CheckMenuItem& item = _miFaderWindows.emplace_back(state->name);
    item.set_active(state->isActive);
    item.signal_toggled().connect([&]() { FaderWindow(*state); });
    item.show();
    _menuFaderWindows.append(item);
  }
}

}  // namespace glight::gui
