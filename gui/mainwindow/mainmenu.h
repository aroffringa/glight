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
  sigc::signal<void()> TheatreDimensions;

  // View menu
  sigc::signal<void()> ShowFixtures;
  sigc::signal<void()> ShowBeams;
  sigc::signal<void()> ShowProjections;
  sigc::signal<void()> ShowStageBorders;
  sigc::signal<void()> FullScreen;

  // Window menu
  sigc::signal<void()> NewFaderWindow;
  sigc::signal<void()> FixtureList;
  sigc::signal<void()> FixtureTypes;
  sigc::signal<void()> SideBar;
  sigc::signal<void()> PowerMonitor;
  sigc::signal<void(bool active)> SceneWindow;
  sigc::signal<void(FaderSetState& fader_set)> FaderWindow;

  bool ShowFixturesActive() const { return _miShowFixtures.get_active(); }
  void SetShowFixtures(bool show_fixtures) {
    _miShowFixtures.set_active(show_fixtures);
  }

  bool ShowBeamsActive() const { return _miShowBeams.get_active(); }
  void SetShowBeams(bool show_beams) { _miShowBeams.set_active(show_beams); }

  bool ShowProjectionsActive() const { return _miShowProjections.get_active(); }
  void SetShowProjections(bool show_projections) {
    _miShowProjections.set_active(show_projections);
  }

  bool ShowStageBordersActive() const {
    return _miShowStageBorders.get_active();
  }
  void SetShowStageBorders(bool show_stage_borders) {
    _miShowStageBorders.set_active(show_stage_borders);
  }

  bool FullScreenActive() const { return _miFullScreen.get_active(); }

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

  bool PowerMonitorActive() const { return _miPowerMonitor.get_active(); }
  void SetPowerMonitorActive(bool active) {
    _miPowerMonitor.set_active(active);
  }

  void SetSceneWindowActive(bool active) { _miSceneWindow.set_active(active); }

  void SetFaderList(const std::vector<std::unique_ptr<FaderSetState>>& faders);

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
  Gtk::SeparatorMenuItem _miDesignSep2;
  Gtk::MenuItem _miTheatreDimensions{"Theatre dimensions..."};

  Gtk::Menu _menuView;
  Gtk::MenuItem _miView{"_View", true};
  Gtk::CheckMenuItem _miShowFixtures{"Show fixtures"};
  Gtk::CheckMenuItem _miShowBeams{"Show beams"};
  Gtk::CheckMenuItem _miShowProjections{"Show projections"};
  Gtk::CheckMenuItem _miShowStageBorders{"Show theatre walls"};
  Gtk::SeparatorMenuItem _miViewSeperator;
  Gtk::CheckMenuItem _miFullScreen{"Full screen"};

  Gtk::Menu _menuWindow;
  Gtk::MenuItem _miWindow{"_Window", true};
  Gtk::CheckMenuItem _miSideBar{"Side bar"};
  Gtk::CheckMenuItem _miPowerMonitor{"Power monitor"};
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
