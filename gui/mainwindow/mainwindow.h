#ifndef GUI_MAIN_WINDOW_H_
#define GUI_MAIN_WINDOW_H_

#include "mainmenu.h"

#include "gui/eventtransmitter.h"
#include "gui/fixtureselection.h"
#include "gui/state/guistate.h"
#include "theatre/forwards.h"

#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/revealer.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/window.h>

#include <sigc++/signal.h>

#include <vector>

namespace glight::system::midi {
class Manager;
}

namespace glight::gui {

class DesignWizard;
class FaderWindow;
class FixtureListWindow;
class FixtureTypesWindow;
class ObjectListFrame;
class PropertiesWindow;
class SceneWindow;
class SettingsWindow;
class VisualizationWidget;

/**
 * @author Andre Offringa
 */
class MainWindow : public Gtk::Window, public EventTransmitter {
 public:
  MainWindow();
  ~MainWindow();

  void EmitUpdate() final override;

  sigc::signal<void()> &SignalUpdateControllables() final override {
    return _signalUpdateControllables;
  }

  void OpenFile(const std::string &filename);

  theatre::Folder &SelectedFolder() const;

  std::unique_ptr<DesignWizard> &GetDesignWizard() { return _designWizard; }

  PropertiesWindow &OpenPropertiesWindow(theatre::FolderObject &object);

  system::midi::Manager &GetMidiManager() { return *midi_manager_; }

  MainMenu &Menu() { return main_menu_; }

 private:
  void InitializeMenu();

  void onFixtureListButtonClicked();
  void onFixtureTypesButtonClicked();
  void onSideBarButtonClicked();

  /**
   * If stateOrNull is nullptr, the first inactive state is selected, or
   * if no states are inactive, a new state is created.
   */
  void addFaderWindow(FaderSetState *stateOrNull = nullptr);

  void increaseManualBeat(int val);
  bool onKeyDown(GdkEventKey *event);
  bool onKeyUp(GdkEventKey *event);
  bool onDelete(GdkEventAny *event);

  void onHideFixtureList();
  void onHideFixtureTypes();

  void onMINewClicked();
  void onMIOpenClicked();
  void onMISaveClicked();
  void onMIImportClicked();
  void onMISettingsClicked();
  void onMIQuitClicked();

  void onMIBlackOutAndDryMode();
  void onMIBlackOut();
  void onMIProtectBlackOut();

  void onMIDesignWizardClicked();

  void onFullscreen();
  void onFaderWindowHidden(FaderWindow *window);
  void onFaderListChange();
  void onFaderWindowSelected(FaderSetState &state);
  FaderWindow *getFaderWindow(const FaderSetState &state);
  void onSceneWindowClicked(bool active);
  void onHideSceneWindow();

  size_t nextControlKeyRow() const;

  void UpdateLayoutLock();

  Gtk::VBox _box;
  Gtk::HBox revealer_box_;

  std::vector<std::unique_ptr<FaderWindow>> _faderWindows;
  std::unique_ptr<FixtureListWindow> _fixtureListWindow;
  std::unique_ptr<FixtureTypesWindow> _fixtureTypesWindow;
  std::unique_ptr<DesignWizard> _designWizard;
  std::unique_ptr<SceneWindow> _sceneWindow;

  std::unique_ptr<theatre::Management> _management;

  Gtk::Revealer revealer_;
  std::unique_ptr<ObjectListFrame> _objectListFrame;
  std::unique_ptr<VisualizationWidget> _visualizationWidget;
  std::unique_ptr<SettingsWindow> settings_window_;

  GUIState _state;
  FixtureSelection _fixtureSelection;

  sigc::signal<void()> _signalUpdateControllables;
  MainMenu main_menu_;
  std::unique_ptr<system::midi::Manager> midi_manager_;
};

}  // namespace glight::gui

#endif
