#ifndef GUI_MAIN_WINDOW_H_
#define GUI_MAIN_WINDOW_H_

#include "mainmenu.h"

#include "../eventtransmitter.h"
#include "../fixtureselection.h"
#include "../state/guistate.h"
#include "../../theatre/forwards.h"

#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/window.h>

#include <sigc++/signal.h>

#include <vector>

namespace glight::gui {

class DesignWizard;
class FaderWindow;
class FixtureListWindow;
class FixtureTypesWindow;
class ObjectListFrame;
class PropertiesWindow;
class SceneWindow;
class VisualizationWindow;

/**
 * @author Andre Offringa
 */
class MainWindow : public Gtk::Window, public EventTransmitter {
 public:
  MainWindow(std::unique_ptr<theatre::DmxDevice> dmxDevice);
  ~MainWindow();

  void EmitUpdate() final override;

  GUIState &State() { return _state; }

  sigc::signal<void()> &SignalUpdateControllables() final override {
    return _signalUpdateControllables;
  }

  theatre::Management &GetManagement() const { return *_management; }

  void OpenFile(const std::string &filename);

  theatre::Folder &SelectedFolder() const;

  std::unique_ptr<DesignWizard> &GetDesignWizard() { return _designWizard; }

  PropertiesWindow &OpenPropertiesWindow(theatre::FolderObject &object);

 private:
  void InitializeMenu();

  void onFixtureListButtonClicked();
  void onFixtureTypesButtonClicked();
  void onVisualizationWindowButtonClicked();

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
  void onHideVisualizationWindow();

  void onMINewClicked();
  void onMIOpenClicked();
  void onMISaveClicked();
  void onMIImportClicked();
  void onMIQuitClicked();

  void onMIBlackOutAndDryMode();
  void onMIBlackOut();
  void onMIProtectBlackOut();

  void onMIDesignWizardClicked();

  void onFaderWindowHidden(FaderWindow *window);
  void onFaderListChange();
  void onFaderWindowSelected(Gtk::CheckMenuItem &menuItem,
                             FaderSetState &state);
  FaderWindow *getFaderWindow(FaderSetState &state);
  void onSceneWindowClicked();
  void onHideSceneWindow();

  size_t nextControlKeyRow() const;

  Gtk::VBox _box;

  std::vector<std::unique_ptr<FaderWindow>> _faderWindows;
  std::unique_ptr<FixtureListWindow> _fixtureListWindow;
  std::unique_ptr<FixtureTypesWindow> _fixtureTypesWindow;
  std::unique_ptr<VisualizationWindow> _visualizationWindow;
  std::unique_ptr<DesignWizard> _designWizard;

  std::unique_ptr<theatre::Management> _management;

  std::unique_ptr<ObjectListFrame> _objectListFrame;
  std::unique_ptr<SceneWindow> _sceneWindow;

  GUIState _state;
  FixtureSelection _fixtureSelection;

  sigc::signal<void()> _signalUpdateControllables;
  MainMenu main_menu_;
};

}  // namespace glight::gui

#endif
