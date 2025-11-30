#include "mainwindow.h"

#include <filesystem>
#include <fstream>
#include <memory>

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/messagedialog.h>

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

MainWindow::MainWindow() : main_menu_(*this) {
  set_title("Glight - show");
  set_default_icon_name("glight");
  set_default_size(800, 500);

  Glib::RefPtr<Gtk::IconTheme> iconTheme = Gtk::IconTheme::create();
  std::filesystem::path iconPath =
      std::filesystem::path(GLIGHT_INSTALL_PATH) / "share/icons";
  iconTheme->add_search_path(iconPath.string());

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
  _objectListFrame->set_vexpand(true);
  _objectListFrame->set_hexpand(false);
  revealer_.set_child(*_objectListFrame);
  revealer_.set_reveal_child(true);
  revealer_.set_transition_type(Gtk::RevealerTransitionType::SLIDE_RIGHT);
  revealer_.set_vexpand(true);
  revealer_.set_hexpand(false);
  revealer_box_.append(revealer_);
  revealer_box_.set_vexpand(true);

  right_box_.append(power_monitor_);
  right_box_.set_expand(true);
  power_monitor_.Start();

  _visualizationWidget = std::make_unique<VisualizationWidget>(
      _management.get(), this, &_fixtureSelection, this);
  _visualizationWidget->set_expand(true);
  _visualizationWidget->add_controller(GetKeyController());
  right_box_.append(*_visualizationWidget);
  revealer_box_.append(right_box_);

  _box.append(revealer_box_);

  set_child(_box);
  power_monitor_.set_visible(false);

  add_controller(GetKeyController());
  signal_close_request().connect(sigc::mem_fun(*this, &MainWindow::onDelete),
                                 false);
}

MainWindow::~MainWindow() {
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

  main_menu_.LockLayout.connect([&](bool) { UpdateLayoutLock(); });
  main_menu_.BlackOut.connect([&]() { onMIBlackOut(); });
  main_menu_.DesignWizard.connect([&]() { onMIDesignWizardClicked(); });
  main_menu_.TheatreDimensions.connect(
      [&]() { onMITheatreDimensionsClicked(); });

  main_menu_.ShowFixtures.connect([&](bool new_value) {
    _visualizationWidget->SetDrawFixtures(new_value);
    _state.SetShowFixtures(new_value);
    _visualizationWidget->Update();
  });
  main_menu_.ShowBeams.connect([&](bool new_value) {
    _visualizationWidget->SetDrawBeams(new_value);
    _state.SetShowBeams(new_value);
    _visualizationWidget->Update();
  });
  main_menu_.ShowProjections.connect([&](bool new_value) {
    _visualizationWidget->SetDrawProjections(new_value);
    _state.SetShowProjections(new_value);
    _visualizationWidget->Update();
  });
  main_menu_.ShowStageBorders.connect([&](bool new_value) {
    _visualizationWidget->SetDrawBorders(new_value);
    _state.SetShowStageBorders(new_value);
    _visualizationWidget->Update();
  });

  main_menu_.SideBar.connect([&](bool) { onSideBarButtonClicked(); });
  main_menu_.PowerMonitor.connect([&](bool) { onPowerMonitorButtonClicked(); });
  main_menu_.FullScreen.connect([&](bool) { onFullscreen(); });
  main_menu_.NewFaderWindow.connect([&]() { addFaderWindow(); });
  main_menu_.FixtureList.connect([&](bool) { onFixtureListButtonClicked(); });
  main_menu_.FixtureTypes.connect([&](bool) { onFixtureTypesButtonClicked(); });
  main_menu_.SceneWindow.connect(
      [&](bool active) { onSceneWindowClicked(active); });
  main_menu_.FaderWindow.connect(
      [&](FaderSetState &fader_set) { onFaderWindowSelected(fader_set); });

  _box.append(main_menu_);
}

std::shared_ptr<Gtk::EventController> MainWindow::GetKeyController() {
  std::shared_ptr<Gtk::EventControllerKey> key_controller =
      Gtk::EventControllerKey::create();
  key_controller->signal_key_pressed().connect(
      [&](guint keyval, guint keycode, Gdk::ModifierType state) {
        return MainWindow::onKeyDown(keyval);
      },
      false);
  key_controller->signal_key_released().connect(
      [&](guint keyval, guint keycode, Gdk::ModifierType state) {
        MainWindow::onKeyUp(keyval);
      });
  return key_controller;
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
  newWindow->add_controller(GetKeyController());
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
    window.add_controller(GetKeyController());
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
    window.add_controller(GetKeyController());
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

bool MainWindow::onKeyDown(guint keyval) {
  if (keyval == '0')
    increaseManualBeat(0);
  else if (keyval == '1')
    increaseManualBeat(1);
  else if (keyval == '2')
    increaseManualBeat(2);
  else if (keyval == '3')
    increaseManualBeat(3);
  else if (keyval == '4')
    increaseManualBeat(4);
  else if (keyval == GDK_KEY_Escape) {
    // Swap ?
  } else if (keyval == GDK_KEY_BackSpace) {
    // black out
  } else {
    bool handled = false;
    for (std::unique_ptr<FaderWindow> &cw : _faderWindows)
      if (!handled) handled = cw->HandleKeyDown(keyval);
    return !handled;
  }
  return false;
}

bool MainWindow::onKeyUp(guint keyval) {
  bool handled = false;
  for (std::unique_ptr<FaderWindow> &cw : _faderWindows)
    if (!handled) handled = cw->HandleKeyUp(keyval);
  return handled;
}

bool MainWindow::onDelete() {
  if (_management->IsEmpty())
    hide();
  else {
    dialog_ = std::make_unique<Gtk::MessageDialog>(
        *this, "Are you sure you want to close glight?", false,
        Gtk::MessageType::QUESTION, Gtk::ButtonsType::OK_CANCEL);
    auto &dialog = static_cast<Gtk::MessageDialog &>(*dialog_);
    dialog.set_secondary_text("All lights will be stopped.");
    dialog.signal_response().connect([this](int response) {
      if (response == Gtk::ResponseType::OK) hide();
      dialog_.reset();
    });
    dialog.show();
  }
  return true;
}

void MainWindow::LoadMenuOptionsFromState() {
  main_menu_.SetLayoutLocked(_state.LayoutLocked());
  main_menu_.SetShowFixtures(_state.ShowFixtures());
  main_menu_.SetShowBeams(_state.ShowBeams());
  main_menu_.SetShowProjections(_state.ShowProjections());
  main_menu_.SetShowStageBorders(_state.ShowStageBorders());
}

void MainWindow::NewShow() {
  Instance::Selection().SetSelection({});
  std::unique_lock<std::mutex> lock(_management->Mutex());
  _management->Clear();
  _faderWindows.clear();
  child_windows_.Clear();
  _state.Clear();

  lock.unlock();

  // It's important to sent an update now, because windows might have
  // references to removed fixtures.
  EmitUpdate();
  LoadMenuOptionsFromState();

  addFaderWindow();
}

void MainWindow::onMINewClicked() {
  if (_management->IsEmpty())
    NewShow();
  else {
    dialog_ = std::make_unique<Gtk::MessageDialog>(
        *this, "Are you sure you want to start a new show?", false,
        Gtk::MessageType::QUESTION, Gtk::ButtonsType::OK_CANCEL);
    Gtk::MessageDialog &dialog = static_cast<Gtk::MessageDialog &>(*dialog_);
    dialog.set_secondary_text("All lights will be stopped.");
    dialog.signal_response().connect([this](int response) {
      if (response == Gtk::ResponseType::OK) NewShow();
      dialog_.reset();
    });
    dialog.show();
  }
}

void MainWindow::OpenFile(const std::string &filename) {
  Instance::Selection().SetSelection({});
  std::unique_lock<std::mutex> lock(_management->Mutex());
  _management->Clear();
  _faderWindows.clear();
  _state.Clear();
  child_windows_.Clear();

  system::Read(filename, *_management, &_state);

  lock.unlock();

  EmitUpdate();
  set_default_size(_state.WindowWidth(), _state.WindowHeight());
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

void MainWindow::Open() {
  dialog_ = std::make_unique<Gtk::FileChooserDialog>(
      *this, "Open glight show", Gtk::FileChooser::Action::OPEN);
  Gtk::FileChooserDialog &dialog =
      static_cast<Gtk::FileChooserDialog &>(*dialog_);
  dialog.add_button("Cancel", Gtk::ResponseType::CANCEL);
  dialog.add_button("Open", Gtk::ResponseType::OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("Glight show (*.gshow)");
  filter->add_pattern("*.gshow");
  filter->add_mime_type("text/gshow+json");
  dialog.add_filter(filter);

  dialog.signal_response().connect([this](int response) {
    if (response == Gtk::ResponseType::OK) {
      Gtk::FileChooserDialog &dialog =
          static_cast<Gtk::FileChooserDialog &>(*dialog_);
      const std::string filename = dialog.get_file()->get_path();
      // resetting the dialog will delete the lambda, so store this beforehand.
      MainWindow &me = *this;
      dialog_.reset();
      me.OpenFile(filename);
    } else {
      dialog_.reset();
    }
  });
  dialog.show();
}

void MainWindow::onMIOpenClicked() {
  if (_management->IsEmpty())
    Open();
  else {
    dialog_ = std::make_unique<Gtk::MessageDialog>(
        *this, "Are you sure you want to open a new show?", false,
        Gtk::MessageType::QUESTION, Gtk::ButtonsType::OK_CANCEL);
    Gtk::MessageDialog &dialog = static_cast<Gtk::MessageDialog &>(*dialog_);
    dialog.set_secondary_text("Lights will change to the new show.");
    dialog.signal_response().connect([this](int response) {
      // Open will reset dialog_, so we should not reset it in that case
      if (response == Gtk::ResponseType::OK)
        Open();
      else
        dialog_.reset();
    });
    dialog.show();
  }
}

void MainWindow::onMISaveClicked() {
  dialog_ = std::make_unique<Gtk::FileChooserDialog>(
      *this, "Save glight show", Gtk::FileChooser::Action::SAVE);
  Gtk::FileChooserDialog &dialog =
      static_cast<Gtk::FileChooserDialog &>(*dialog_);

  dialog.add_button("Cancel", Gtk::ResponseType::CANCEL);
  dialog.add_button("Save", Gtk::ResponseType::OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("Glight show (*.gshow)");
  filter->add_pattern("*.gshow");
  filter->add_mime_type("text/gshow+json");
  dialog.add_filter(filter);
  dialog.signal_response().connect([this](int response) {
    if (response == Gtk::ResponseType::OK) {
      Gtk::FileChooserDialog &dialog =
          static_cast<Gtk::FileChooserDialog &>(*dialog_);
      Glib::ustring filename(dialog.get_file()->get_path());
      if (filename.find('.') == Glib::ustring::npos) filename += ".gshow";
      _state.SetWindowDimensions(get_width(), get_height());
      std::lock_guard<std::mutex> lock(_management->Mutex());
      system::Write(filename, *_management, &_state);
    }
    dialog_.reset();
  });
  dialog.show();
}

void MainWindow::onMIImportClicked() {
  dialog_ = std::make_unique<Gtk::FileChooserDialog>(
      *this, "Import fixture types", Gtk::FileChooser::Action::OPEN);
  Gtk::FileChooserDialog &dialog =
      static_cast<Gtk::FileChooserDialog &>(*dialog_);

  dialog.add_button("Cancel", Gtk::ResponseType::CANCEL);
  dialog.add_button("Open", Gtk::ResponseType::OK);

  Glib::RefPtr<Gtk::FileFilter> json_filter = Gtk::FileFilter::create();
  json_filter->set_name("Open fixture file (*.json)");
  json_filter->add_pattern("*.json");
  json_filter->add_mime_type("text/json");
  dialog.add_filter(json_filter);

  Glib::RefPtr<Gtk::FileFilter> gshow_filter = Gtk::FileFilter::create();
  gshow_filter->set_name("Glight file format (*.gshow)");
  gshow_filter->add_pattern("*.gshow");
  dialog.add_filter(gshow_filter);

  dialog.signal_response().connect([this, gshow_filter](int response) {
    if (response == Gtk::ResponseType::OK) {
      Gtk::FileChooserDialog &dialog =
          static_cast<Gtk::FileChooserDialog &>(*dialog_);
      const std::string filename = dialog.get_file()->get_path();

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
      dialog_.reset();
    }
  });
  dialog.show();
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
    child_windows_.Open<SceneWindow>(
        [this]() { main_menu_.SetSceneWindowActive(false); }, *this);
  } else {
    child_windows_.Hide<SceneWindow>();
  }
}

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
