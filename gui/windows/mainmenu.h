#ifndef GUI_MAIN_MENU_H_
#define GUI_MAIN_MENU_H_

#include <gtkmm/checkmenuitem.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/separatormenuitem.h>

#include <memory>
#include <vector>

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
  sigc::signal<void()> Quit;

  // Design menu
  sigc::signal<void()> BlackOut;
  sigc::signal<void()> DesignWizard;

  // Window menu
  sigc::signal<void()> NewFaderWindow;
  sigc::signal<void()> FixtureList;
  sigc::signal<void()> FixtureTypes;
  sigc::signal<void()> Visualization;
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

  bool VisualizationActive() const {
    return _miVisualizationWindow.get_active();
  }
  void SetVisualizationActive(bool active) {
    _miVisualizationWindow.set_active(active);
  }

  void SetSceneWindowActive(bool active) { _miSceneWindow.set_active(active); }

  void SetFaderList(const std::vector<std::unique_ptr<FaderSetState>>& faders);

 private:
  Gtk::Menu _menuFile, _menuDesign, _menuWindow, _menuFaderWindows;

  Gtk::MenuItem _miFile, _miDesign, _miWindow;
  Gtk::MenuItem _miNew, _miOpen, _miSave, _miImport, _miQuit;
  Gtk::MenuItem _miBlackOut;
  Gtk::CheckMenuItem _miProtectBlackout;
  Gtk::SeparatorMenuItem _miDesignSep1, _miDesignSep2;
  Gtk::MenuItem _miDesignWizard;
  Gtk::CheckMenuItem _miFixtureListWindow;
  Gtk::CheckMenuItem _miFixtureTypesWindow;
  Gtk::MenuItem _miFaderWindowMenu;
  Gtk::MenuItem _miNewFaderWindow;
  Gtk::SeparatorMenuItem _miFaderWindowSeperator;
  std::vector<Gtk::CheckMenuItem> _miFaderWindows;
  Gtk::CheckMenuItem _miVisualizationWindow;
  Gtk::CheckMenuItem _miSceneWindow;
};

}  // namespace glight::gui

#endif
