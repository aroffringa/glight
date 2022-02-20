#ifndef SHOWWINDOW_H
#define SHOWWINDOW_H

#include "../eventtransmitter.h"
#include "../fixtureselection.h"
#include "../guistate.h"

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/notebook.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/window.h>

#include <sigc++/signal.h>

#include <vector>

/**
        @author Andre Offringa
*/
class ShowWindow : public Gtk::Window, public EventTransmitter {
 public:
  ShowWindow(std::unique_ptr<class DmxDevice> dmxDevice);
  ~ShowWindow();

  void EmitUpdate() final override;

  GUIState &State() { return _state; }

  sigc::signal<void(Management &)> &SignalChangeManagement() final override {
    return _signalChangeManagement;
  }
  sigc::signal<void()> &SignalUpdateControllables() final override {
    return _signalUpdateControllables;
  }

  Management &GetManagement() const { return *_management; }

  void OpenFile(const std::string &filename);

  std::string Path();

  std::unique_ptr<class DesignWizard> &GetDesignWizard() {
    return _designWizard;
  }

 private:
  void onConfigurationWindowButtonClicked();
  void onVisualizationWindowButtonClicked();

  /**
   * If stateOrNull is nullptr, the first inactive state is selected, or
   * if no states are inactive, a new state is created.
   */
  void addFaderWindow(FaderSetupState *stateOrNull = nullptr);

  void increaseManualBeat(int val);
  bool onKeyDown(GdkEventKey *event);
  bool onKeyUp(GdkEventKey *event);
  bool onDelete(GdkEventAny *event);

  void onHideFixtureList();
  void onHideVisualizationWindow();

  void createMenu();

  void onMINewClicked();
  void onMIOpenClicked();
  void onMISaveClicked();
  void onMIQuitClicked();

  void updateDryModeState();
  void onMIDryModeClicked();
  void onMICancelDryModeClicked();
  void onMISwapModesClicked();
  void onMIRecoverClicked();
  void onMIBlackOutAndDryMode();
  void onMIBlackOut();
  void onMIProtectBlackOut();

  void onMIDesignWizardClicked();

  void onFaderWindowHidden(class FaderWindow *window);
  void onFaderListChange();
  void onFaderWindowSelected(Gtk::CheckMenuItem &menuItem,
                             FaderSetupState &state);
  FaderWindow *getFaderWindow(FaderSetupState &state);

  void changeManagement(class Management *newManagement,
                        bool moveControlSliders);

  size_t nextControlKeyRow() const;

  Gtk::VBox _box;

  std::vector<std::unique_ptr<class FaderWindow>> _faderWindows;
  std::unique_ptr<class FixtureListWindow> _fixtureListWindow;
  std::unique_ptr<class VisualizationWindow> _visualizationWindow;
  std::unique_ptr<class DesignWizard> _designWizard;

  std::unique_ptr<class Management> _management;
  /**
   * When running in dry mode, the running management is moved here
   * and kept running, while all actions from then on affect the
   * dry mode management that is not connected to a device.
   */
  std::unique_ptr<class Management> _backgroundManagement;
  class PresetCollection *_preset;

  std::unique_ptr<class ObjectListFrame> _objectListFrame;
  std::unique_ptr<class SceneFrame> _sceneFrame;

  GUIState _state;
  FixtureSelection _fixtureSelection;

  sigc::signal<void(Management &)> _signalChangeManagement;
  sigc::signal<void()> _signalUpdateControllables;

  Gtk::Menu _menuFile, _menuDesign, _menuWindow, _menuFaderWindows;

  Gtk::MenuItem _miFile, _miDesign, _miWindow;
  Gtk::ImageMenuItem _miNew, _miOpen, _miSave, _miQuit;
  Gtk::CheckMenuItem _miDryMode;
  Gtk::MenuItem _miCancelDryMode;
  Gtk::MenuItem _miSwapModes;
  Gtk::MenuItem _miRecover;
  Gtk::MenuItem _miBlackOutAndDryMode;
  Gtk::MenuItem _miBlackOut;
  Gtk::CheckMenuItem _miProtectBlackout;
  Gtk::SeparatorMenuItem _miDesignSep1, _miDesignSep2;
  Gtk::MenuItem _miDesignWizard;
  Gtk::CheckMenuItem _miFixtureListWindow;
  Gtk::MenuItem _miFaderWindowMenu, _miNewFaderWindow;
  Gtk::SeparatorMenuItem _miFaderWindowSeperator;
  std::vector<Gtk::CheckMenuItem> _miFaderWindows;
  Gtk::CheckMenuItem _miVisualizationWindow;

  Gtk::MenuBar _menuBar;
  Gtk::Notebook _notebook;
};

#endif
