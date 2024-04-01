#ifndef GUI_MAIN_MENU_H_
#define GUI_MAIN_MENU_H_

#include <memory>
#include <vector>

#include <gtkmm/checkmenuitem.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/separatormenuitem.h>

#include "theatre/effecttype.h"

namespace glight::gui {

class FaderSetState;

class MainMenu : public Gtk::MenuBar {
 public:
  MainMenu();

  // File menu
  sigc::signal<void()> New;
  sigc::signal<void()> Open;
  sigc::signal<void()> Save;
  sigc::signal<void()> Import;
  sigc::signal<void()> Settings;
  sigc::signal<void()> Quit;

  // Design menu
  sigc::signal<void()> LockLayout;
  sigc::signal<void()> BlackOut;

  sigc::signal<void()> AddEmptyPreset;
  sigc::signal<void()> AddCurrentPreset;
  sigc::signal<void()> AddChase;
  sigc::signal<void()> AddTimeSequence;
  sigc::signal<void(theatre::EffectType)> AddEffect;
  sigc::signal<void()> AddFolder;
  sigc::signal<void()> DeleteObject;

  sigc::signal<void()> DesignWizard;

  // Window menu
  sigc::signal<void()> FullScreen;
  sigc::signal<void()> NewFaderWindow;
  sigc::signal<void()> FixtureList;
  sigc::signal<void()> FixtureTypes;
  sigc::signal<void()> SideBar;
  sigc::signal<void(bool active)> SceneWindow;
  sigc::signal<void(FaderSetState& fader_set)> FaderWindow;

  bool FixtureListActive() const { return _miFixtureListWindow.get_active(); }
  void SetFixtureListActive(bool active) {
    _miFixtureListWindow.set_active(active);
  }

  bool FixtureTypesActive() const { return _miFixtureTypesWindow.get_active(); }
  void SetFixtureTypesActive(bool active) {
    _miFixtureTypesWindow.set_active(active);
  }

  bool SideBarActive() const { return _miSideBar.get_active(); }
  void SetSideBarActive(bool active) { _miSideBar.set_active(active); }

  void SetSceneWindowActive(bool active) { _miSceneWindow.set_active(active); }

  void SetFaderList(const std::vector<std::unique_ptr<FaderSetState>>& faders);

  bool FullScreenActive() const { return _miFullScreen.get_active(); }

  bool IsLayoutLocked() const { return _miLockLayout.get_active(); }
  void SetLayoutLocked(bool lock) { _miLockLayout.set_active(lock); }

  void SetIsObjectSelected(bool is_selected) {
    _miDeleteObject.set_sensitive(is_selected);
  }

 private:
  Gtk::Menu file_menu_;
  Gtk::MenuItem _miFile{"_File", true};
  Gtk::MenuItem _miNew{"New"};
  Gtk::MenuItem _miOpen{"_Open...", true};
  Gtk::MenuItem _miSave{"Save _as...", true};
  Gtk::MenuItem _miImport{"_Import fixtures...", true};
  Gtk::SeparatorMenuItem file_sep1_mi_;
  Gtk::MenuItem settings_mi_{"_Settings...", true};
  Gtk::SeparatorMenuItem file_sep2_mi_;
  Gtk::MenuItem _miQuit{"_Quit", true};

  Gtk::Menu _menuDesign;
  Gtk::MenuItem _miDesign{"_Design", true};
  Gtk::CheckMenuItem _miLockLayout{"Lock layout"};
  Gtk::CheckMenuItem _miProtectBlackout{"Protect black-out"};
  Gtk::MenuItem _miBlackOut{"Black-out"};
  Gtk::SeparatorMenuItem _miDesignSep1;
  Gtk::Menu preset_sub_menu_;
  Gtk::MenuItem _miAddPreset{"Add preset"};
  Gtk::MenuItem _miAddEmptyPreset{"Empty"};
  Gtk::MenuItem _miAddCurrentPreset{"From current"};
  Gtk::MenuItem _miAddChase{"Add chase"};
  Gtk::MenuItem _miAddSequence{"Add sequence"};
  Gtk::MenuItem _miAddEffect{"Add effect"};
  Gtk::Menu effect_sub_menu_;
  std::vector<Gtk::MenuItem> effect_menu_items_;
  Gtk::MenuItem _miAddFolder{"Add folder"};
  Gtk::MenuItem _miDeleteObject{"Delete"};
  Gtk::MenuItem _miDesignWizard{"Design wizard..."};

  Gtk::Menu _menuWindow;
  Gtk::MenuItem _miWindow{"_Window", true};
  Gtk::CheckMenuItem _miSideBar{"Side bar"};
  Gtk::CheckMenuItem _miFullScreen{"Full screen"};
  Gtk::CheckMenuItem _miFixtureListWindow{"Fixtures"};
  Gtk::CheckMenuItem _miFixtureTypesWindow{"Fixture types"};
  Gtk::Menu _menuFaderWindows;
  Gtk::MenuItem _miFaderWindowMenu{"Fader windows"};
  Gtk::MenuItem _miNewFaderWindow{"New"};
  Gtk::SeparatorMenuItem _miFaderWindowSeperator;
  std::vector<Gtk::CheckMenuItem> _miFaderWindows;
  Gtk::CheckMenuItem _miSceneWindow{"Scene"};
};

}  // namespace glight::gui

#endif
