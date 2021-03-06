#include "showwindow.h"

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "designwizard.h"
#include "fixturelistwindow.h"
#include "objectlistframe.h"
#include "sceneframe.h"
#include "visualizationwindow.h"

#include "faders/faderwindow.h"

#include "../theatre/dmxdevice.h"
#include "../theatre/fixture.h"
#include "../theatre/management.h"
#include "../theatre/presetcollection.h"
#include "../theatre/show.h"
#include "../theatre/theatre.h"

#include "../system/reader.h"
#include "../system/writer.h"

ShowWindow::ShowWindow(std::unique_ptr<DmxDevice> device)
    : _miFile("_File", true),
      _miDesign("_Design", true),
      _miWindow("_Window", true),
      _miNew(Gtk::Stock::NEW),
      _miOpen(Gtk::Stock::OPEN),
      _miSave(Gtk::Stock::SAVE_AS),
      _miQuit(Gtk::Stock::QUIT),
      _miDryMode("Dry mode"),
      _miCancelDryMode("Cancel dry mode"),
      _miSwapModes("Swap modes"),
      _miRecover("Recover"),
      _miBlackOutAndDryMode("Black-out with dry mode"),
      _miBlackOut("Black-out"),
      _miProtectBlackout("Protect black-out"),
      _miDesignWizard("Design wizard"),
      _miFixtureListWindow("Fixture list"),
      _miFaderWindowMenu("Fader windows"),
      _miNewFaderWindow("New"),
      _miVisualizationWindow("Visualization") {
  set_title("Glight - show");

  _management.reset(new Management());
  _management->AddDevice(std::move(device));
  _management->StartBeatFinder();

  _management->Run();

  _fixtureListWindow.reset(
      new FixtureListWindow(this, *_management, &_fixtureSelection));
  _fixtureListWindow->signal_key_press_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyDown));
  _fixtureListWindow->signal_key_release_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyUp));
  _fixtureListWindow->signal_hide().connect([&]() { onHideFixtureList(); });

  _visualizationWindow.reset(new VisualizationWindow(_management.get(), this,
                                                     &_fixtureSelection, this));
  _visualizationWindow->signal_key_press_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyDown));
  _visualizationWindow->signal_key_release_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyUp));
  _visualizationWindow->signal_hide().connect(
      [&]() { onHideVisualizationWindow(); });

  createMenu();

  _state.FaderSetupSignalChange().connect([&]() { onFaderListChange(); });
  addFaderWindow();

  _objectListFrame.reset(new ObjectListFrame(*_management, *this));
  _sceneFrame.reset(new SceneFrame(*_management, *this));

  _notebook.append_page(*_objectListFrame, "Objects");
  _notebook.append_page(*_sceneFrame, "Timeline");

  _box.pack_start(_notebook);

  add(_box);
  _box.show_all();

  signal_key_press_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyDown));
  signal_key_release_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyUp));
  signal_delete_event().connect(sigc::mem_fun(*this, &ShowWindow::onDelete));
}

ShowWindow::~ShowWindow() {
  _menuFile.detach();

  _sceneFrame.reset();
  _visualizationWindow.reset();
  _fixtureListWindow.reset();

  _faderWindows.clear();

  _management.reset();
}

void ShowWindow::EmitUpdate() {
  _sceneFrame->Update();
  _signalUpdateControllables();
}

void ShowWindow::addFaderWindow(FaderSetupState *stateOrNull) {
  if (stateOrNull == nullptr) {
    for (std::unique_ptr<FaderSetupState> &setup : _state.FaderSetups()) {
      if (!setup->isActive) {
        stateOrNull = setup.get();
        break;
      }
    }
  }
  _faderWindows.emplace_back(std::make_unique<FaderWindow>(
      *this, _state, *_management, nextControlKeyRow()));
  FaderWindow *newWindow = _faderWindows.back().get();
  if (stateOrNull == nullptr)
    newWindow->LoadNew();
  else
    newWindow->LoadState(stateOrNull);
  newWindow->signal_key_press_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyDown));
  newWindow->signal_key_release_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyUp));
  newWindow->signal_hide().connect(sigc::bind(
      sigc::mem_fun(*this, &ShowWindow::onFaderWindowHidden), newWindow));
  newWindow->show();
  _state.EmitFaderSetupChangeSignal();
}

void ShowWindow::onConfigurationWindowButtonClicked() {
  bool show = _miFixtureListWindow.get_active();
  if (show)
    _fixtureListWindow->show();
  else
    _fixtureListWindow->hide();
}

void ShowWindow::onVisualizationWindowButtonClicked() {
  bool show = _miVisualizationWindow.get_active();
  if (show)
    _visualizationWindow->show();
  else
    _visualizationWindow->hide();
}

void ShowWindow::increaseManualBeat(int val) {
  _management->IncreaseManualBeat(val);
  if (_backgroundManagement) _backgroundManagement->IncreaseManualBeat(val);
}

bool ShowWindow::onKeyDown(GdkEventKey *event) {
  if (event->keyval == '0')
    increaseManualBeat(0);
  else if (event->keyval == '1')
    increaseManualBeat(1);
  else if (event->keyval == '2')
    increaseManualBeat(2);
  else if (event->keyval == '3')
    increaseManualBeat(3);
  else if (event->keyval == '4')
    increaseManualBeat(4);
  else if (event->keyval == GDK_KEY_Escape) {
    if (_miDryMode.get_active()) {
      onMISwapModesClicked();
    } else {
      _miDryMode.set_active(true);
      onMIDryModeClicked();
    }
  } else if (event->keyval == GDK_KEY_BackSpace) {
    if (_miDryMode.get_active()) {
      _miDryMode.set_active(false);
      onMIDryModeClicked();
    } else {
      if (!_miProtectBlackout.get_active()) {
        onMIBlackOutAndDryMode();
      }
    }
  } else {
    if (_sceneFrame->HandleKeyDown(event->keyval)) return true;
    bool handled = false;
    for (std::unique_ptr<FaderWindow> &cw : _faderWindows)
      if (!handled) handled = cw->HandleKeyDown(event->keyval);
    return !handled;
  }
  return false;
}

bool ShowWindow::onKeyUp(GdkEventKey *event) {
  bool handled = false;
  for (std::unique_ptr<FaderWindow> &cw : _faderWindows)
    if (!handled) handled = cw->HandleKeyUp(event->keyval);
  return handled;
}

bool ShowWindow::onDelete(GdkEventAny *) {
  if (_management->IsEmpty())
    return false;
  else {
    Gtk::MessageDialog dialog(*this, "Are you sure you want to close glight?",
                              false, Gtk::MESSAGE_QUESTION,
                              Gtk::BUTTONS_OK_CANCEL);
    dialog.set_secondary_text("All lights will be stopped.");
    int result = dialog.run();
    return result != Gtk::RESPONSE_OK;
  }
}

void ShowWindow::createMenu() {
  _menuFile.set_title("_File");

  _miNew.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMINewClicked));
  _menuFile.append(_miNew);

  _miOpen.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMIOpenClicked));
  _menuFile.append(_miOpen);

  _miSave.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMISaveClicked));
  _menuFile.append(_miSave);

  _miQuit.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMIQuitClicked));
  _menuFile.append(_miQuit);

  _miFile.set_submenu(_menuFile);
  _menuBar.append(_miFile);

  _menuDesign.set_title("_Design");

  _miDryMode.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMIDryModeClicked));
  _menuDesign.append(_miDryMode);

  _miCancelDryMode.set_sensitive(false);
  _miCancelDryMode.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMICancelDryModeClicked));
  _menuDesign.append(_miCancelDryMode);

  _miSwapModes.set_sensitive(false);
  _miSwapModes.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMISwapModesClicked));
  _menuDesign.append(_miSwapModes);

  _miRecover.set_sensitive(false);
  _miRecover.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMIRecoverClicked));
  _menuDesign.append(_miRecover);

  _menuDesign.append(_miDesignSep1);

  _miProtectBlackout.signal_activate().connect(
      [&]() { onMIProtectBlackOut(); });
  _miProtectBlackout.set_active(true);
  _menuDesign.append(_miProtectBlackout);

  _miBlackOutAndDryMode.signal_activate().connect(
      [&]() { onMIBlackOutAndDryMode(); });
  _miBlackOutAndDryMode.set_sensitive(false);
  _menuDesign.append(_miBlackOutAndDryMode);

  _miBlackOut.signal_activate().connect([&]() { onMIBlackOut(); });
  _miBlackOut.set_sensitive(false);
  _menuDesign.append(_miBlackOut);

  _menuDesign.append(_miDesignSep2);

  _miDesignWizard.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMIDesignWizardClicked));
  _menuDesign.append(_miDesignWizard);

  _miDesign.set_submenu(_menuDesign);
  _menuBar.append(_miDesign);

  _menuWindow.set_title("_Window");

  _miNewFaderWindow.signal_activate().connect([&]() { addFaderWindow(); });
  _menuFaderWindows.append(_miNewFaderWindow);

  _menuFaderWindows.append(_miFaderWindowSeperator);

  _miFaderWindowMenu.set_submenu(_menuFaderWindows);
  _menuWindow.append(_miFaderWindowMenu);

  _miFixtureListWindow.set_active(false);
  _miFixtureListWindow.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onConfigurationWindowButtonClicked));
  _menuWindow.append(_miFixtureListWindow);

  _miVisualizationWindow.set_active(false);
  _miVisualizationWindow.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onVisualizationWindowButtonClicked));
  _menuWindow.append(_miVisualizationWindow);

  _miWindow.set_submenu(_menuWindow);
  _menuBar.append(_miWindow);

  _box.pack_start(_menuBar, false, false);
}

void ShowWindow::onMINewClicked() {
  bool confirmed = false;
  if (_management->IsEmpty())
    confirmed = true;
  else {
    Gtk::MessageDialog dialog(
        *this, "Are you sure you want to start a new show?", false,
        Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
    dialog.set_secondary_text("All lights will be stopped.");
    int result = dialog.run();
    confirmed = (result == Gtk::RESPONSE_OK);
  }

  if (confirmed) {
    std::unique_lock<std::mutex> lock(_management->Mutex());
    _management->Clear();
    _state.Clear();
    lock.unlock();

    EmitUpdate();

    _faderWindows.clear();
    addFaderWindow();
  }
}

void ShowWindow::OpenFile(const std::string &filename) {
  std::unique_lock<std::mutex> lock(_management->Mutex());
  _management->Clear();
  _faderWindows.clear();
  _state.Clear();
  Reader reader(*_management);
  reader.SetGUIState(_state);
  reader.Read(filename);

  if (_management->Show().Scenes().size() != 0)
    _sceneFrame->SetSelectedScene(*_management->Show().Scenes()[0]);
  else
    _sceneFrame->SetNoSelectedScene();

  lock.unlock();

  EmitUpdate();

  if (_state.Empty()) {
    std::cout << "File did not contain GUI state info: will start with default "
                 "faders.\n";
    addFaderWindow();
  } else {
    for (const std::unique_ptr<FaderSetupState> &state : _state.FaderSetups()) {
      if (state->isActive) {
        // Currently it is not displayed, so to avoid the control window doing
        // the wrong thing, isActive is set to false and will be set to true by
        // the control window.
        state->isActive = false;
        addFaderWindow(state.get());
      }
    }
  }
  _state.EmitFaderSetupChangeSignal();
}

void ShowWindow::onMIOpenClicked() {
  bool confirmed = false;
  if (_management->IsEmpty())
    confirmed = true;
  else {
    Gtk::MessageDialog dialog(
        *this, "Are you sure you want to open a new show?", false,
        Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
    dialog.set_secondary_text("Lights will change to the new show.");
    int result = dialog.run();
    confirmed = (result == Gtk::RESPONSE_OK);
  }

  if (confirmed) {
    Gtk::FileChooserDialog dialog(*this, "Open glight show",
                                  Gtk::FILE_CHOOSER_ACTION_OPEN);

    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button("Open", Gtk::RESPONSE_OK);

    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    filter->set_name("Glight show");
    filter->add_pattern("*.gshow");
    filter->add_mime_type("text/gshow+xml");
    dialog.add_filter(filter);

    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) OpenFile(dialog.get_filename());
  }
}

void ShowWindow::onMISaveClicked() {
  Gtk::FileChooserDialog dialog(*this, "Save glight show",
                                Gtk::FILE_CHOOSER_ACTION_SAVE);

  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Save", Gtk::RESPONSE_OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("Glight show");
  filter->add_pattern("*.gshow");
  filter->add_mime_type("text/gshow+xml");
  dialog.add_filter(filter);

  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK) {
    Glib::ustring filename(dialog.get_filename());
    if (filename.find('.') == Glib::ustring::npos) filename += ".gshow";

    std::lock_guard<std::mutex> lock(_management->Mutex());
    Writer writer(*_management);
    writer.SetGUIState(_state);
    writer.Write(filename);
  }
}

void ShowWindow::onMIQuitClicked() { hide(); }

void ShowWindow::updateDryModeState() {
  bool dryMode = _miDryMode.get_active();
  _miCancelDryMode.set_sensitive(dryMode);
  _miSwapModes.set_sensitive(dryMode);
  _miRecover.set_sensitive(dryMode);
  _miBlackOutAndDryMode.set_sensitive(!dryMode &&
                                      !_miProtectBlackout.get_active());
  _miOpen.set_sensitive(!dryMode);
  _miSave.set_sensitive(!dryMode);
}

void ShowWindow::onMIDryModeClicked() {
  Folder &activeFolder = _objectListFrame->SelectedFolder();
  std::string path = activeFolder.FullPath();
  bool enterDryMode = _miDryMode.get_active();
  if (enterDryMode && _backgroundManagement == nullptr) {
    // Switch from real mode to dry mode
    _backgroundManagement = std::move(_management);
    _management = _backgroundManagement->MakeDryMode();
    _management->Run();
    _visualizationWindow->SetDryMode(_management.get());
    changeManagement(_management.get(), false);
  } else if (!enterDryMode && _backgroundManagement != nullptr) {
    // Switch from dry mode to real mode
    _management->SwapDevices(*_backgroundManagement);
    _visualizationWindow->MakeDryModeReal();
    _backgroundManagement.reset();
  }
  updateDryModeState();
  FolderObject &folder = _management->GetObjectFromPath(path);
  _objectListFrame->OpenFolder(static_cast<Folder &>(folder));
}

void ShowWindow::onMICancelDryModeClicked() {
  if (_miDryMode.get_active() && _backgroundManagement != nullptr) {
    Gtk::MessageDialog dialog(
        *this, "Are you sure you want to cancel dry mode?", false,
        Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
    dialog.set_secondary_text(
        "All changes made after entering dry mode will be lost");
    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) {
      Folder &activeFolder = _objectListFrame->SelectedFolder();
      std::string path = activeFolder.FullPath();

      std::swap(_backgroundManagement, _management);
      _visualizationWindow->SetRealMode();
      changeManagement(_management.get(), true);
      _backgroundManagement.reset();
      _miDryMode.set_active(false);
      updateDryModeState();

      FolderObject &folder = _management->GetObjectFromPath(path);
      _objectListFrame->OpenFolder(static_cast<Folder &>(folder));
    }
  }
}

void ShowWindow::onMISwapModesClicked() {
  if (_miDryMode.get_active() && _backgroundManagement != nullptr) {
    Folder &activeFolder = _objectListFrame->SelectedFolder();
    std::string path = activeFolder.FullPath();

    _management->SwapDevices(*_backgroundManagement);
    std::swap(_backgroundManagement, _management);
    _visualizationWindow->MakeDryModeReal();
    _visualizationWindow->SetDryMode(_management.get());
    changeManagement(_management.get(), true);

    updateDryModeState();
    FolderObject &folder = _management->GetObjectFromPath(path);
    _objectListFrame->OpenFolder(static_cast<Folder &>(folder));
  }
}

void ShowWindow::onMIRecoverClicked() {
  if (_backgroundManagement) {
    _management->Recover(*_backgroundManagement);
    for (std::unique_ptr<FaderWindow> &fw : _faderWindows) fw->ReloadValues();
  }
}

void ShowWindow::onMIBlackOutAndDryMode() {
  _miDryMode.set_active(false);
  Folder &activeFolder = _objectListFrame->SelectedFolder();
  std::string path = activeFolder.FullPath();
  if (!_miDryMode.get_active() && _backgroundManagement == nullptr) {
    // Switch from real mode to dry mode
    _backgroundManagement = std::move(_management);
    _management = _backgroundManagement->MakeDryMode();
    // Black out real management
    _backgroundManagement->BlackOut();
    _management->Run();
    _visualizationWindow->SetDryMode(_management.get());
    changeManagement(_management.get(), true);

    _miDryMode.set_active(true);
  }
  updateDryModeState();
  FolderObject &folder = _management->GetObjectFromPath(path);
  _objectListFrame->OpenFolder(static_cast<Folder &>(folder));
}

void ShowWindow::changeManagement(Management *newManagement,
                                  bool moveControlSliders) {
  _state.ChangeManagement(*newManagement);
  _signalChangeManagement(*newManagement);
  for (std::unique_ptr<FaderWindow> &cw : _faderWindows)
    cw->ChangeManagement(*newManagement, moveControlSliders);
  _sceneFrame->ChangeManagement(*newManagement);
  EmitUpdate();
}

void ShowWindow::onFaderWindowHidden(FaderWindow *window) {
  for (std::vector<std::unique_ptr<FaderWindow>>::iterator i =
           _faderWindows.begin();
       i != _faderWindows.end(); ++i) {
    if (i->get() == window) {
      _faderWindows.erase(i);
      break;
    }
  }
}

void ShowWindow::onFaderListChange() {
  _miFaderWindows.clear();

  for (const std::unique_ptr<FaderSetupState> &state : _state.FaderSetups()) {
    _miFaderWindows.emplace_back(state->name);
    _miFaderWindows.back().set_active(state->isActive);
    _miFaderWindows.back().signal_toggled().connect(
        [&]() { onFaderWindowSelected(_miFaderWindows.back(), *state); });
    _miFaderWindows.back().show();
    _menuFaderWindows.append(_miFaderWindows.back());
  }
}

FaderWindow *ShowWindow::getFaderWindow(FaderSetupState &state) {
  for (const std::unique_ptr<FaderWindow> &window : _faderWindows) {
    if (window->State() == &state) return window.get();
  }
  return nullptr;
}

void ShowWindow::onFaderWindowSelected(Gtk::CheckMenuItem &menuItem,
                                       FaderSetupState &state) {
  FaderWindow *window = getFaderWindow(state);
  if (window) {
    window->hide();
  } else {
    addFaderWindow(&state);
  }
}

size_t ShowWindow::nextControlKeyRow() const {
  size_t index = 0;
  while (index < std::numeric_limits<size_t>::max()) {
    bool found = false;
    for (const std::unique_ptr<FaderWindow> &cw : _faderWindows) {
      if (index == cw->KeyRowIndex()) {
        ++index;
        found = true;
        break;
      }
    }
    if (!found) return index;
  }
  throw std::runtime_error("Error in nextControlKeyRow()");
}

std::string ShowWindow::Path() {
  return _objectListFrame->SelectedFolder().FullPath();
}

void ShowWindow::onMIDesignWizardClicked() {
  if (!_designWizard || !_designWizard->is_visible())
    _designWizard.reset(new DesignWizard(*_management, *this, Path()));
  _designWizard->SetDestinationPath(Path());
  _designWizard->present();
}

void ShowWindow::onMIBlackOut() {
  _management->BlackOut();
  for (std::unique_ptr<FaderWindow> &fw : _faderWindows) fw->ReloadValues();
}

void ShowWindow::onMIProtectBlackOut() {
  bool protect = _miProtectBlackout.get_active();
  _miBlackOut.set_sensitive(!protect);
  _miBlackOutAndDryMode.set_sensitive(!protect);
}

void ShowWindow::onHideFixtureList() { _miFixtureListWindow.set_active(false); }

void ShowWindow::onHideVisualizationWindow() {
  _miVisualizationWindow.set_active(false);
}
