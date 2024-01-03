#include "mainmenu.h"

#include "../state/fadersetstate.h"

namespace glight::gui {

MainMenu::MainMenu()
    : _miFile("_File", true),
      _miDesign("_Design", true),
      _miWindow("_Window", true),
      _miNew("New"),
      _miOpen("_Open...", true),
      _miSave("Save _as...", true),
      _miImport("_Import fixtures...", true),
      _miQuit("_Quit", true),
      _miBlackOut("Black-out"),
      _miLockLayout("Lock layout"),
      _miProtectBlackout("Protect black-out"),
      _miDesignWizard("Design wizard"),
      _miSideBar("Side bar"),
      _miFullScreen("Full screen"),
      _miFixtureListWindow("Fixtures"),
      _miFixtureTypesWindow("Fixture types"),
      _miFaderWindowMenu("Fader windows"),
      _miNewFaderWindow("New"),
      _miSceneWindow("Scene") {
  _miNew.signal_activate().connect(New);
  _menuFile.append(_miNew);

  _miOpen.signal_activate().connect(Open);
  _menuFile.append(_miOpen);

  _miSave.signal_activate().connect(Save);
  _menuFile.append(_miSave);

  _miImport.signal_activate().connect(Import);
  _menuFile.append(_miImport);

  _miQuit.signal_activate().connect(Quit);
  _menuFile.append(_miQuit);

  _miFile.set_submenu(_menuFile);
  append(_miFile);

  _menuDesign.append(_miDesignSep1);

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

  _menuDesign.append(_miDesignSep2);

  _miDesignWizard.signal_activate().connect(DesignWizard);
  _menuDesign.append(_miDesignWizard);

  _miDesign.set_submenu(_menuDesign);
  append(_miDesign);

  _miSideBar.set_active(true);
  _miSideBar.signal_activate().connect(SideBar);
  _menuWindow.append(_miSideBar);

  _miFullScreen.signal_activate().connect(FullScreen);
  _menuWindow.append(_miFullScreen);

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
