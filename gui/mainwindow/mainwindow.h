#ifndef GUI_MAIN_WINDOW_H_
#define GUI_MAIN_WINDOW_H_

#include "mainmenu.h"

#include "gui/eventtransmitter.h"
#include "gui/fixtureselection.h"
#include "gui/components/powermonitor.h"
#include "gui/state/guistate.h"
#include "gui/windows/childwindowlist.h"
#include "theatre/forwards.h"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/notebook.h>
#include <gtkmm/revealer.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/window.h>

#include <sigc++/signal.h>

#include <vector>

namespace glight::system::midi {
class Manager;
}

namespace glight::gui::windows {
class FixtureListWindow;
}

namespace glight::gui {

class DesignWizard;
class FaderWindow;
class ObjectListFrame;
class PropertiesWindow;
class VisualizationWidget;

/**
 * @author Andre Offringa
 */
class MainWindow : public Gtk::ApplicationWindow, public EventTransmitter {
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
  std::shared_ptr<Gtk::EventController> GetKeyController();

  void onFixtureListButtonClicked();
  void onFixtureTypesButtonClicked();
  void onSideBarButtonClicked();
  void onPowerMonitorButtonClicked();

  /**
   * If stateOrNull is nullptr, the first inactive state is selected, or
   * if no states are inactive, a new state is created.
   */
  void addFaderWindow(FaderSetState *stateOrNull = nullptr);

  void increaseManualBeat(int val);
  bool onKeyDown(guint keyval);
  bool onKeyUp(guint keyval);
  bool onDelete();

  void NewShow();
  void onMINewClicked();
  void Open();
  void onMIOpenClicked();
  void onMISaveClicked();
  void onMIImportClicked();

  void onMIBlackOutAndDryMode();
  void onMIBlackOut();
  void onMIProtectBlackOut();

  void onMIDesignWizardClicked();
  void onMITheatreDimensionsClicked();

  void onFullscreen();
  void onFaderWindowHidden(FaderWindow *window);
  void onFaderListChange();
  void onFaderWindowSelected(FaderSetState &state);
  FaderWindow *getFaderWindow(const FaderSetState &state);
  void onSceneWindowClicked(bool active);

  size_t nextControlKeyRow() const;

  void UpdateLayoutLock();
  void LoadMenuOptionsFromState();

  Gtk::Box _box{Gtk::Orientation::VERTICAL};
  Gtk::Box revealer_box_;
  Gtk::Box right_box_{Gtk::Orientation::VERTICAL};

  std::vector<std::unique_ptr<FaderWindow>> _faderWindows;
  std::unique_ptr<DesignWizard> _designWizard;
  windows::ChildWindowList child_windows_;

  std::unique_ptr<theatre::Management> _management;

  components::PowerMonitor power_monitor_;
  Gtk::Revealer revealer_;
  std::unique_ptr<ObjectListFrame> _objectListFrame;
  std::unique_ptr<VisualizationWidget> _visualizationWidget;

  GUIState _state;
  FixtureSelection _fixtureSelection;

  sigc::signal<void()> _signalUpdateControllables;
  MainMenu main_menu_;
  std::unique_ptr<system::midi::Manager> midi_manager_;
  std::unique_ptr<Gtk::Dialog> dialog_;
};

}  // namespace glight::gui

#endif
