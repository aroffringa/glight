#include "mainwindow.h"

#include <filesystem>
#include <fstream>
#include <memory>

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "gui/instance.h"

#include "gui/components/visualizationwidget.h"

#include "gui/faders/faderwindow.h"

#include "gui/mainwindow/objectlistframe.h"

#include "gui/windows/designwizard.h"
#include "gui/windows/fixturelistwindow.h"
#include "gui/windows/fixturetypeswindow.h"
#include "gui/windows/scenewindow.h"
#include "gui/windows/settingswindow.h"
#include "gui/windows/theatredimensions.h"

#include "theatre/fixture.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"
#include "theatre/theatre.h"

#include "system/openfixturereader.h"
#include "system/reader.h"
#include "system/settings.h"
#include "system/writer.h"

#include "system/midi/manager.h"

namespace glight::gui {

MainWindow::MainWindow() {
  set_title("Glight - show");
  set_default_icon_name("glight");
  set_default_size(800, 500);

  Glib::RefPtr<Gtk::IconTheme> iconTheme = Gtk::IconTheme::get_default();
  std::filesystem::path iconPath =
      std::filesystem::path(GLIGHT_INSTALL_PATH) / "share/icons";
  iconTheme->prepend_search_path(iconPath.string());

  Instance::Get().SetState(_state);
  Instance::Get().SetSelection(_fixtureSelection);
  Instance::Get().SetEvents(*this);
  Instance::Settings() = system::LoadSettings();
  _management = std::make_unique<theatre::Management>(Instance::Settings());
  Instance::Get().SetManagement(*_management);
  _management->StartBeatFinder();
  _management->GetUniverses().Open();

  _management->Run();

  midi_manager_ = std::make_unique<system::midi::Manager>();

  InitializeMenu();

  _state.FaderSetSignalChange().connect([&]() { onFaderListChange(); });
  addFaderWindow();

  _objectListFrame = std::make_unique<ObjectListFrame>(*this);
  revealer_.add(*_objectListFrame);
  revealer_.set_reveal_child(true);
  revealer_.set_transition_type(
      Gtk::RevealerTransitionType::REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
  revealer_box_.pack_start(revealer_, false, false);

  right_box_.pack_start(power_monitor_, false, false);
  power_monitor_.Start();

  _visualizationWidget = std::make_unique<VisualizationWidget>(
      _management.get(), this, &_fixtureSelection, this);
  _visualizationWidget->signal_key_press_event().connect(
      sigc::mem_fun(*this, &MainWindow::onKeyDown));
  _visualizationWidget->signal_key_release_event().connect(
      sigc::mem_fun(*this, &MainWindow::onKeyUp));
  right_box_.pack_start(*_visualizationWidget, true, true);
  revealer_box_.pack_end(right_box_, true, true);

  _box.pack_start(revealer_box_, true, true);

  add(_box);
  _box.show_all();
  power_monitor_.set_visible(false);

  signal_key_press_event().connect(
      sigc::mem_fun(*this, &MainWindow::onKeyDown));
  signal_key_release_event().connect(
      sigc::mem_fun(*this, &MainWindow::onKeyUp));
  signal_delete_event().connect(sigc::mem_fun(*this, &MainWindow::onDelete));
}

MainWindow::~MainWindow() {
  _sceneWindow.reset();
  _visualizationWidget.reset();
  child_windows_.Clear();

  _faderWindows.clear();

  _management.reset();

  system::Save(Instance::Settings());
}

void MainWindow::InitializeMenu() {
  main_menu_.New.connect(sigc::mem_fun(*this, &MainWindow::onMINewClicked));
  main_menu_.Open.connect(sigc::mem_fun(*this, &MainWindow::onMIOpenClicked));
  main_menu_.Save.connect(sigc::mem_fun(*this, &MainWindow::onMISaveClicked));

  main_menu_.Import.connect(
      sigc::mem_fun(*this, &MainWindow::onMIImportClicked));
  main_menu_.Settings.connect(
      [&]() { child_windows_.Open<windows::SettingsWindow>(); });
  main_menu_.Quit.connect([&]() { hide(); });

  main_menu_.LockLayout.connect([&]() { UpdateLayoutLock(); });
  main_menu_.BlackOut.connect([&]() { onMIBlackOut(); });
  main_menu_.DesignWizard.connect([&]() { onMIDesignWizardClicked(); });
  main_menu_.TheatreDimensions.connect(
      [&]() { onMITheatreDimensionsClicked(); });

  main_menu_.ShowFixtures.connect([&]() {
    _visualizationWidget->SetDrawFixtures(main_menu_.ShowFixturesActive());
    _state.SetShowFixtures(main_menu_.ShowFixturesActive());
    _visualizationWidget->Update();
  });
  main_menu_.ShowBeams.connect([&]() {
    _visualizationWidget->SetDrawBeams(main_menu_.ShowBeamsActive());
    _state.SetShowBeams(main_menu_.ShowBeamsActive());
    _visualizationWidget->Update();
  });
  main_menu_.ShowProjections.connect([&]() {
    _visualizationWidget->SetDrawProjections(
        main_menu_.ShowProjectionsActive());
    _state.SetShowProjections(main_menu_.ShowProjectionsActive());
    _visualizationWidget->Update();
  });
  main_menu_.ShowStageBorders.connect([&]() {
    _visualizationWidget->SetDrawBorders(main_menu_.ShowStageBordersActive());
    _state.SetShowStageBorders(main_menu_.ShowStageBordersActive());
    _visualizationWidget->Update();
  });

  main_menu_.SideBar.connect([&]() { onSideBarButtonClicked(); });
  main_menu_.PowerMonitor.connect([&]() { onPowerMonitorButtonClicked(); });
  main_menu_.FullScreen.connect([&]() { onFullscreen(); });
  main_menu_.NewFaderWindow.connect([&]() { addFaderWindow(); });
  main_menu_.FixtureList.connect(
      sigc::mem_fun(*this, &MainWindow::onFixtureListButtonClicked));
  main_menu_.FixtureTypes.connect(
      sigc::mem_fun(*this, &MainWindow::onFixtureTypesButtonClicked));
  main_menu_.SceneWindow.connect(
      [&](bool active) { onSceneWindowClicked(active); });
  main_menu_.FaderWindow.connect(
      [&](FaderSetState &fader_set) { onFaderWindowSelected(fader_set); });

  _box.pack_start(main_menu_, false, false);
}

void MainWindow::EmitUpdate() { _signalUpdateControllables(); }

void MainWindow::addFaderWindow(FaderSetState *stateOrNull) {
  _faderWindows.emplace_back(
      std::make_unique<FaderWindow>(nextControlKeyRow()));
  FaderWindow *newWindow = _faderWindows.back().get();
  if (_faderWindows.size() == 1 && midi_manager_->GetNFaders() != 0) {
    newWindow->SetMidiManager(*midi_manager_);
  }
  if (stateOrNull == nullptr)
    newWindow->LoadNew();
  else
    newWindow->LoadState(stateOrNull);
  newWindow->signal_key_press_event().connect(
      sigc::mem_fun(*this, &MainWindow::onKeyDown));
  newWindow->signal_key_release_event().connect(
      sigc::mem_fun(*this, &MainWindow::onKeyUp));
  newWindow->signal_hide().connect(sigc::bind(
      sigc::mem_fun(*this, &MainWindow::onFaderWindowHidden), newWindow));
  newWindow->show();
  _state.EmitFaderSetChangeSignal();
}

void MainWindow::onFixtureListButtonClicked() {
  const bool show = main_menu_.FixtureListActive();
  if (show) {
    windows::FixtureListWindow &window =
        child_windows_.Open<windows::FixtureListWindow>(
            [&]() { main_menu_.SetFixtureListActive(false); });
    window.signal_key_press_event().connect(
        sigc::mem_fun(*this, &MainWindow::onKeyDown));
    window.signal_key_release_event().connect(
        sigc::mem_fun(*this, &MainWindow::onKeyUp));
  } else {
    child_windows_.Hide<windows::FixtureListWindow>();
  }
}

void MainWindow::onFixtureTypesButtonClicked() {
  const bool show = main_menu_.FixtureTypesActive();
  if (show) {
    windows::FixtureTypesWindow &window =
        child_windows_.Open<windows::FixtureTypesWindow>(
            [&]() { main_menu_.SetFixtureTypesActive(false); });
    window.signal_key_press_event().connect(
        sigc::mem_fun(*this, &MainWindow::onKeyDown));
    window.signal_key_release_event().connect(
        sigc::mem_fun(*this, &MainWindow::onKeyUp));
  } else {
    child_windows_.Hide<windows::FixtureTypesWindow>();
  }
}

void MainWindow::onSideBarButtonClicked() {
  if (revealer_.get_child_revealed() != main_menu_.SideBarActive()) {
    revealer_.set_reveal_child(main_menu_.SideBarActive());
  }
}

void MainWindow::onPowerMonitorButtonClicked() {
  power_monitor_.set_visible(main_menu_.PowerMonitorActive());
}

void MainWindow::increaseManualBeat(int val) {
  _management->IncreaseManualBeat(val);
}

bool MainWindow::onKeyDown(GdkEventKey *event) {
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

bool MainWindow::onKeyUp(GdkEventKey *event) {
  bool handled = false;
  for (std::unique_ptr<FaderWindow> &cw : _faderWindows)
    if (!handled) handled = cw->HandleKeyUp(event->keyval);
  return handled;
}

bool MainWindow::onDelete(GdkEventAny * /*unused*/) {
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

void MainWindow::LoadMenuOptionsFromState() {
  main_menu_.SetLayoutLocked(_state.LayoutLocked());
  main_menu_.SetShowFixtures(_state.ShowFixtures());
  main_menu_.SetShowBeams(_state.ShowBeams());
  main_menu_.SetShowProjections(_state.ShowProjections());
  main_menu_.SetShowStageBorders(_state.ShowStageBorders());
}

void MainWindow::onMINewClicked() {
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
    Instance::Selection().SetSelection({});
    std::unique_lock<std::mutex> lock(_management->Mutex());
    _management->Clear();
    _faderWindows.clear();
    _sceneWindow.reset();
    _state.Clear();
    lock.unlock();

    // It's important to sent an update now, because windows might have
    // references to removed fixtures.
    EmitUpdate();
    LoadMenuOptionsFromState();

    addFaderWindow();
  }
}

void MainWindow::OpenFile(const std::string &filename) {
  Instance::Selection().SetSelection({});
  std::unique_lock<std::mutex> lock(_management->Mutex());
  _management->Clear();
  _faderWindows.clear();
  _state.Clear();
  _sceneWindow.reset();

  system::Read(filename, *_management, &_state);

  lock.unlock();

  EmitUpdate();
  resize(_state.WindowWidth(), _state.WindowHeight());
  move(_state.WindowPositionX(), _state.WindowPositionY());
  LoadMenuOptionsFromState();
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

  _state.EmitFaderSetChangeSignal();
}

void MainWindow::onMIOpenClicked() {
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

    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("Open", Gtk::RESPONSE_OK);

    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    filter->set_name("Glight show (*.gshow)");
    filter->add_pattern("*.gshow");
    filter->add_mime_type("text/gshow+json");
    dialog.add_filter(filter);

    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) OpenFile(dialog.get_filename());
  }
}

void MainWindow::onMISaveClicked() {
  Gtk::FileChooserDialog dialog(*this, "Save glight show",
                                Gtk::FILE_CHOOSER_ACTION_SAVE);

  dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Save", Gtk::RESPONSE_OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("Glight show (*.gshow)");
  filter->add_pattern("*.gshow");
  filter->add_mime_type("text/gshow+json");
  dialog.add_filter(filter);

  int result = dialog.run();
  if (result == Gtk::RESPONSE_OK) {
    Glib::ustring filename(dialog.get_filename());
    if (filename.find('.') == Glib::ustring::npos) filename += ".gshow";
    int x;
    int y;
    get_position(x, y);
    _state.SetWindowPosition(x, y, get_width(), get_height());
    std::lock_guard<std::mutex> lock(_management->Mutex());
    system::Write(filename, *_management, &_state);
  }
}

void MainWindow::onMIImportClicked() {
  Gtk::FileChooserDialog dialog(*this, "Import fixture types",
                                Gtk::FILE_CHOOSER_ACTION_OPEN);

  dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  Glib::RefPtr<Gtk::FileFilter> json_filter = Gtk::FileFilter::create();
  json_filter->set_name("Open fixture file (*.json)");
  json_filter->add_pattern("*.json");
  json_filter->add_mime_type("text/json");
  dialog.add_filter(json_filter);

  Glib::RefPtr<Gtk::FileFilter> gshow_filter = Gtk::FileFilter::create();
  gshow_filter->set_name("Glight file format (*.gshow)");
  gshow_filter->add_pattern("*.gshow");
  dialog.add_filter(gshow_filter);

  const int result = dialog.run();
  if (result == Gtk::RESPONSE_OK) {
    const std::string filename = dialog.get_filename();

    if (dialog.get_filter() == gshow_filter) {
      std::lock_guard<std::mutex> lock(_management->Mutex());
      system::ImportFixtureTypes(filename, *_management);
    } else {
      std::ifstream stream(filename);
      std::unique_ptr<json::Node> root = json::Parse(stream);
      std::lock_guard<std::mutex> lock(_management->Mutex());
      system::ReadOpenFixture(*_management, *root);
    }
    EmitUpdate();
  }
}

void MainWindow::onFaderWindowHidden(FaderWindow *window) {
  for (std::vector<std::unique_ptr<FaderWindow>>::iterator i =
           _faderWindows.begin();
       i != _faderWindows.end(); ++i) {
    if (i->get() == window) {
      _faderWindows.erase(i);
      break;
    }
  }
}

void MainWindow::onFaderListChange() {
  main_menu_.SetFaderList(_state.FaderSets());
}

FaderWindow *MainWindow::getFaderWindow(const FaderSetState &state) {
  for (const std::unique_ptr<FaderWindow> &window : _faderWindows) {
    if (window->State() == &state) return window.get();
  }
  return nullptr;
}

void MainWindow::onFaderWindowSelected(FaderSetState &state) {
  FaderWindow *window = getFaderWindow(state);
  if (window) {
    window->hide();
  } else {
    addFaderWindow(&state);
  }
}

size_t MainWindow::nextControlKeyRow() const {
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

theatre::Folder &MainWindow::SelectedFolder() const {
  return _objectListFrame->SelectedFolder();
}

void MainWindow::onMIDesignWizardClicked() {
  if (!_designWizard || !_designWizard->is_visible()) {
    _designWizard = std::make_unique<DesignWizard>();
  }
  _designWizard->SetCurrentPath(SelectedFolder().FullPath());
  _designWizard->present();
}

void MainWindow::onMITheatreDimensionsClicked() {
  child_windows_.Open<windows::TheatreDimensions>();
}

void MainWindow::onMIBlackOut() {
  _management->BlackOut(false, 0.0);
  for (std::unique_ptr<FaderWindow> &fw : _faderWindows) fw->UpdateValues();
}

PropertiesWindow &MainWindow::OpenPropertiesWindow(
    theatre::FolderObject &object) {
  return _objectListFrame->OpenPropertiesWindow(object);
}

void MainWindow::onSceneWindowClicked(bool active) {
  if (active) {
    _sceneWindow = std::make_unique<SceneWindow>(*_management, *this, *this);
    _sceneWindow->present();
    _sceneWindow->signal_hide().connect([&]() { onHideSceneWindow(); });
  } else {
    _sceneWindow->hide();
  }
}

void MainWindow::onHideSceneWindow() { main_menu_.SetSceneWindowActive(false); }

void MainWindow::onFullscreen() {
  if (main_menu_.FullScreenActive())
    fullscreen();
  else
    unfullscreen();
}

void MainWindow::UpdateLayoutLock() {
  const bool layout_locked = main_menu_.IsLayoutLocked();
  _state.SetLayoutLocked(layout_locked);
  child_windows_.SetLayoutLocked(layout_locked);
}

}  // namespace glight::gui
