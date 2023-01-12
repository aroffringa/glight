#include "showwindow.h"

#include "fixturelistwindow.h"
#include "fixturetypeswindow.h"
#include "visualizationwindow.h"
#include "scenewindow.h"

#include "../designwizard.h"
#include "../objectlistframe.h"

#include "../faders/faderwindow.h"

#include "../../theatre/dmxdevice.h"
#include "../../theatre/fixture.h"
#include "../../theatre/management.h"
#include "../../theatre/presetcollection.h"
#include "../../theatre/theatre.h"

#include "../../system/openfixturereader.h"
#include "../../system/reader.h"
#include "../../system/writer.h"

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <filesystem>
#include <fstream>

namespace glight::gui {

ShowWindow::ShowWindow(std::unique_ptr<theatre::DmxDevice> device)
    : _miFile("_File", true),
      _miDesign("_Design", true),
      _miWindow("_Window", true),
      _miNew(Gtk::Stock::NEW),
      _miOpen(Gtk::Stock::OPEN),
      _miSave(Gtk::Stock::SAVE_AS),
      _miImport("_Import...", true),
      _miQuit(Gtk::Stock::QUIT),
      _miBlackOut("Black-out"),
      _miProtectBlackout("Protect black-out"),
      _miDesignWizard("Design wizard"),
      _miFixtureListWindow("Fixtures"),
      _miFixtureTypesWindow("Fixture types"),
      _miFaderWindowMenu("Fader windows"),
      _miNewFaderWindow("New"),
      _miVisualizationWindow("Visualization"),
      _miSceneWindow("Scene") {
  set_title("Glight - show");
  set_default_icon_name("glight");

  Glib::RefPtr<Gtk::IconTheme> iconTheme = Gtk::IconTheme::get_default();
  std::filesystem::path iconPath =
      std::filesystem::path(GLIGHT_INSTALL_PATH) / "share/icons";
  iconTheme->prepend_search_path(iconPath.string());

  _management = std::make_unique<theatre::Management>();
  _management->AddDevice(std::move(device));
  _management->StartBeatFinder();

  _management->Run();

  _fixtureListWindow = std::make_unique<glight::gui::FixtureListWindow>(
      *this, *_management, _fixtureSelection);
  _fixtureListWindow->signal_key_press_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyDown));
  _fixtureListWindow->signal_key_release_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyUp));
  _fixtureListWindow->signal_hide().connect([&]() { onHideFixtureList(); });

  _fixtureTypesWindow.reset(new FixtureTypesWindow(this, *_management));
  _fixtureTypesWindow->signal_key_press_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyDown));
  _fixtureTypesWindow->signal_key_release_event().connect(
      sigc::mem_fun(*this, &ShowWindow::onKeyUp));
  _fixtureTypesWindow->signal_hide().connect([&]() { onHideFixtureTypes(); });

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

  _objectListFrame = std::make_unique<ObjectListFrame>(*_management, *this);

  _box.pack_start(*_objectListFrame);

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

  _sceneWindow.reset();
  _visualizationWindow.reset();
  _fixtureListWindow.reset();
  _fixtureTypesWindow.reset();

  _faderWindows.clear();

  _management.reset();
}

void ShowWindow::EmitUpdate() { _signalUpdateControllables(); }

void ShowWindow::addFaderWindow(FaderSetState *stateOrNull) {
  if (stateOrNull == nullptr) {
    for (std::unique_ptr<FaderSetState> &setup : _state.FaderSets()) {
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

void ShowWindow::onFixtureListButtonClicked() {
  const bool show = _miFixtureListWindow.get_active();
  if (show)
    _fixtureListWindow->show();
  else
    _fixtureListWindow->hide();
}

void ShowWindow::onFixtureTypesButtonClicked() {
  const bool show = _miFixtureTypesWindow.get_active();
  if (show)
    _fixtureTypesWindow->show();
  else
    _fixtureTypesWindow->hide();
}

void ShowWindow::onVisualizationWindowButtonClicked() {
  const bool show = _miVisualizationWindow.get_active();
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
    // Swap ?
  } else if (event->keyval == GDK_KEY_BackSpace) {
    // black out
  } else {
    if (_sceneWindow->HandleKeyDown(event->keyval)) return true;
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

  _miImport.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMIImportClicked));
  _menuFile.append(_miImport);

  _miQuit.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onMIQuitClicked));
  _menuFile.append(_miQuit);

  _miFile.set_submenu(_menuFile);
  _menuBar.append(_miFile);

  _menuDesign.set_title("_Design");

  _menuDesign.append(_miDesignSep1);

  _miProtectBlackout.signal_activate().connect(
      [&]() { onMIProtectBlackOut(); });
  _miProtectBlackout.set_active(true);
  _menuDesign.append(_miProtectBlackout);

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
      sigc::mem_fun(*this, &ShowWindow::onFixtureListButtonClicked));
  _menuWindow.append(_miFixtureListWindow);

  _miFixtureTypesWindow.set_active(false);
  _miFixtureTypesWindow.signal_activate().connect(
      sigc::mem_fun(*this, &ShowWindow::onFixtureTypesButtonClicked));
  _menuWindow.append(_miFixtureTypesWindow);

  _miVisualizationWindow.set_active(false);
  _miVisualizationWindow.signal_activate().connect(
      [&]() { onVisualizationWindowButtonClicked(); });
  _menuWindow.append(_miVisualizationWindow);

  _miSceneWindow.set_active(false);
  _miSceneWindow.signal_activate().connect([&]() { onSceneWindowClicked(); });
  _menuWindow.append(_miSceneWindow);

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
    _faderWindows.clear();
    _sceneWindow.reset();
    _state.Clear();
    lock.unlock();

    EmitUpdate();

    addFaderWindow();
  }
}

void ShowWindow::OpenFile(const std::string &filename) {
  std::unique_lock<std::mutex> lock(_management->Mutex());
  _management->Clear();
  _faderWindows.clear();
  _state.Clear();
  _sceneWindow.reset();

  system::Read(filename, *_management, &_state);

  lock.unlock();

  EmitUpdate();

  if (_state.Empty()) {
    std::cout << "File did not contain GUI state info: will start with default "
                 "faders.\n";
    addFaderWindow();
  } else {
    for (const std::unique_ptr<FaderSetState> &state : _state.FaderSets()) {
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
    filter->add_mime_type("text/gshow+json");
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
  filter->add_mime_type("text/gshow+json");
  dialog.add_filter(filter);

  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK) {
    Glib::ustring filename(dialog.get_filename());
    if (filename.find('.') == Glib::ustring::npos) filename += ".gshow";

    std::lock_guard<std::mutex> lock(_management->Mutex());
    system::Write(filename, *_management, &_state);
  }
}

void ShowWindow::onMIImportClicked() {
  Gtk::FileChooserDialog dialog(*this, "Open glight show",
                                Gtk::FILE_CHOOSER_ACTION_OPEN);

  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("Open fixture file");
  filter->add_pattern("*.json");
  filter->add_mime_type("text/json");
  dialog.add_filter(filter);

  const int result = dialog.run();
  if (result == Gtk::RESPONSE_OK) {
    std::ifstream stream(dialog.get_filename());
    std::unique_ptr<json::Node> root = json::Parse(stream);
    {
      std::lock_guard<std::mutex> lock(_management->Mutex());
      system::ReadOpenFixture(*_management, *root);
    }
    EmitUpdate();
  }
}

void ShowWindow::onMIQuitClicked() { hide(); }

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

  for (const std::unique_ptr<FaderSetState> &state : _state.FaderSets()) {
    _miFaderWindows.emplace_back(state->name);
    _miFaderWindows.back().set_active(state->isActive);
    _miFaderWindows.back().signal_toggled().connect(
        [&]() { onFaderWindowSelected(_miFaderWindows.back(), *state); });
    _miFaderWindows.back().show();
    _menuFaderWindows.append(_miFaderWindows.back());
  }
}

FaderWindow *ShowWindow::getFaderWindow(FaderSetState &state) {
  for (const std::unique_ptr<FaderWindow> &window : _faderWindows) {
    if (window->State() == &state) return window.get();
  }
  return nullptr;
}

void ShowWindow::onFaderWindowSelected(Gtk::CheckMenuItem &menuItem,
                                       FaderSetState &state) {
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

theatre::Folder &ShowWindow::SelectedFolder() const {
  return _objectListFrame->SelectedFolder();
}

void ShowWindow::onMIDesignWizardClicked() {
  if (!_designWizard || !_designWizard->is_visible()) {
    _designWizard.reset(new DesignWizard(*_management, *this));
  }
  _designWizard->SetCurrentPath(SelectedFolder().FullPath());
  _designWizard->present();
}

void ShowWindow::onMIBlackOut() {
  _management->BlackOut();
  for (std::unique_ptr<FaderWindow> &fw : _faderWindows) fw->ReloadValues();
}

void ShowWindow::onMIProtectBlackOut() {
  bool protect = _miProtectBlackout.get_active();
  _miBlackOut.set_sensitive(!protect);
}

void ShowWindow::onHideFixtureList() { _miFixtureListWindow.set_active(false); }

void ShowWindow::onHideFixtureTypes() {
  _miFixtureTypesWindow.set_active(false);
}

void ShowWindow::onHideVisualizationWindow() {
  _miVisualizationWindow.set_active(false);
}

PropertiesWindow &ShowWindow::OpenPropertiesWindow(
    theatre::FolderObject &object) {
  return _objectListFrame->OpenPropertiesWindow(object);
}

void ShowWindow::onSceneWindowClicked() {
  const bool show = _miSceneWindow.get_active();
  if (show) {
    _sceneWindow = std::make_unique<SceneWindow>(*_management, *this, *this);
    _sceneWindow->present();
    _sceneWindow->signal_hide().connect([&]() { onHideSceneWindow(); });
  } else {
    _sceneWindow->hide();
  }
}

void ShowWindow::onHideSceneWindow() { _miSceneWindow.set_active(false); }

}  // namespace glight::gui
