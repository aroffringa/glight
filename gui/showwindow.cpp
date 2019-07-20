#include "showwindow.h"

#include <gtkmm/stock.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>

#include "configurationwindow.h"
#include "designwizard.h"
#include "faderwindow.h"
#include "objectlistframe.h"
#include "sceneframe.h"
#include "visualizationwindow.h"

#include "../theatre/dmxdevice.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"
#include "../theatre/fixture.h"
#include "../theatre/presetcollection.h"
#include "../theatre/show.h"

#include "../system/reader.h"
#include "../system/writer.h"

ShowWindow::ShowWindow(std::unique_ptr<DmxDevice> device) :
	_miFile("_File", true),
	_miDesign("_Design", true),
	_miWindow("_Window", true),
	_miNew(Gtk::Stock::NEW),
	_miOpen(Gtk::Stock::OPEN),
	_miSave(Gtk::Stock::SAVE_AS),
	_miQuit(Gtk::Stock::QUIT),
	_miDryMode("Dry mode"),
	_miCancelDryMode("Cancel dry mode"),
	   _miDesignWizard("Design wizard"),
	_miConfigWindow("Fixtures config"),
	_miNewControlWindow("New faders window"),
	_miVisualizationWindow("Visualization")
{
	set_title("Glight - show");

	_management.reset(new Management());
	_management->AddDevice(std::move(device));
	_management->StartBeatFinder();

	_management->Run();

	addFaderWindow();

	_configurationWindow.reset(new ConfigurationWindow(this));
	_configurationWindow->signal_key_press_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyDown));
	_configurationWindow->signal_key_release_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyUp));

	_visualizationWindow.reset(new VisualizationWindow(_management.get(), this));
	
	createMenu();
	
	_objectListFrame.reset(new ObjectListFrame(*_management, *this));
	_sceneFrame.reset(new SceneFrame(*_management, *this));

	_notebook.append_page(*_objectListFrame, "Objects");
	_notebook.append_page(*_sceneFrame, "Timeline");

	_box.pack_start(_notebook);
	
	add(_box);
	_box.show_all();
		
	signal_key_press_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyDown));
	signal_key_release_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyUp));
}


ShowWindow::~ShowWindow()
{
	_menuFile.detach();
	
	_sceneFrame.reset();
	_visualizationWindow.reset();
	_configurationWindow.reset();

	_faderWindows.clear();

	_management.reset();
}

void ShowWindow::EmitUpdate()
{
	for(std::unique_ptr<FaderWindow>& fw : _faderWindows)
		fw->Update();
	_sceneFrame->Update();
	_signalUpdateControllables();
}

void ShowWindow::addFaderWindow(FaderSetupState* stateOrNull)
{
	if(stateOrNull == nullptr)
	{
		for(std::unique_ptr<FaderSetupState>& setup : _state.FaderSetups())
		{
			if(!setup->isActive)
			{
				stateOrNull = setup.get();
				break;
			}
		}
	}
	_faderWindows.emplace_back(new FaderWindow(this, *_management, nextControlKeyRow()));
	FaderWindow *newWindow = _faderWindows.back().get();
	if(stateOrNull == nullptr)
		newWindow->LoadNew();
	else
		newWindow->LoadState(stateOrNull);
	newWindow->signal_key_press_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyDown));
	newWindow->signal_key_release_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyUp));
	newWindow->signal_hide().connect(sigc::bind(sigc::mem_fun(*this, &ShowWindow::onControlWindowHidden), newWindow));
	newWindow->show();
}

void ShowWindow::onConfigurationWindowButtonClicked()
{
	bool show = _miConfigWindow.get_active();
	if(show)
		_configurationWindow->show();
	else
		_configurationWindow->hide();
}

void ShowWindow::onVisualizationWindowButtonClicked()
{
	bool show = _miVisualizationWindow.get_active();
	if(show)
		_visualizationWindow->show();
	else
		_visualizationWindow->hide();
}

bool ShowWindow::onKeyDown(GdkEventKey *event)
{
	if(_sceneFrame->HandleKeyDown(event->keyval))
		return true;
	bool handled = false;
	for(std::unique_ptr<FaderWindow>& cw : _faderWindows)
		if(!handled)
			handled = cw->HandleKeyDown(event->keyval);
	return handled;
}

bool ShowWindow::onKeyUp(GdkEventKey *event)
{
	bool handled = false;
	for(std::unique_ptr<FaderWindow>& cw : _faderWindows)
		if(!handled)
			handled = cw->HandleKeyUp(event->keyval);
	return handled;
}

void ShowWindow::createMenu()
{
	_menuFile.set_title("_File");
	
	_miNew.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onMINewClicked));
	_menuFile.append(_miNew);

	_miOpen.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onMIOpenClicked));
	_menuFile.append(_miOpen);

	_miSave.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onMISaveClicked));
	_menuFile.append(_miSave);
	
	_miQuit.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onMIQuitClicked));
	_menuFile.append(_miQuit);

	_miFile.set_submenu(_menuFile);
	_menuBar.append(_miFile);
	
	_menuDesign.set_title("_Design");
	
	_miDryMode.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onMIDryModeClicked));
	_menuDesign.append(_miDryMode);
		
	_miCancelDryMode.set_sensitive(false);
	_miCancelDryMode.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onMICancelDryModeClicked));
	_menuDesign.append(_miCancelDryMode);
	
	_menuDesign.append(_miDesignSep1);
		
	_miDesignWizard.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onMIDesignWizardClicked));
	_menuDesign.append(_miDesignWizard);
	
	_miDesign.set_submenu(_menuDesign);
	_menuBar.append(_miDesign);
	
	_menuWindow.set_title("_Window");
	
	_miNewControlWindow.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onControlWindowButtonClicked));
	_menuWindow.append(_miNewControlWindow);

	_miConfigWindow.set_active(false);
	_miConfigWindow.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onConfigurationWindowButtonClicked));
	_menuWindow.append(_miConfigWindow);

	_miVisualizationWindow.set_active(false);
	_miVisualizationWindow.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onVisualizationWindowButtonClicked));
	_menuWindow.append(_miVisualizationWindow);
	
	_miWindow.set_submenu(_menuWindow);
	_menuBar.append(_miWindow);
	
	_box.pack_start(_menuBar, false, false);
}

void ShowWindow::onMINewClicked()
{
	std::unique_lock<std::mutex> lock(_management->Mutex());
	_management->Clear();
	lock.unlock();

	EmitUpdate();
}

void ShowWindow::OpenFile(const std::string& filename)
{
	std::unique_lock<std::mutex> lock(_management->Mutex());
	_management->Clear();
	_faderWindows.clear();
	_state.Clear();
	Reader reader(*_management);
	reader.SetGUIState(_state);
	reader.Read(filename);

	if(_management->Show().Scenes().size() != 0)
		_sceneFrame->SetSelectedScene(*_management->Show().Scenes()[0]);
	else
		_sceneFrame->SetNoSelectedScene();
	
	lock.unlock();

	EmitUpdate();
	
	if(_state.Empty())
	{
		std::cout << "File did not contain GUI state info: will start with default faders.\n";
		addFaderWindow();
	}
	else {
		for(const std::unique_ptr<FaderSetupState>& state : _state.FaderSetups())
		{
			if(state->isActive)
			{
				// Currently it is not displayed, so to avoid the control window doing the
				// wrong thing, isActive is set to false and will be set to true by the control window.
				state->isActive = false;
				addFaderWindow(state.get());
			}
		}
	}
}

void ShowWindow::onMIOpenClicked()
{
	Gtk::FileChooserDialog dialog(*this, "Open glight show", Gtk::FILE_CHOOSER_ACTION_OPEN);
	
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button("Open", Gtk::RESPONSE_OK);
	
	Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
	filter->set_name("Glight show");
	filter->add_pattern("*.gshow");
	filter->add_mime_type("text/gshow+xml");
	dialog.add_filter(filter);
	
	int result = dialog.run();
	if(result == Gtk::RESPONSE_OK)
		OpenFile(dialog.get_filename());
}

void ShowWindow::onMISaveClicked()
{
  Gtk::FileChooserDialog dialog(*this, "Save glight show", Gtk::FILE_CHOOSER_ACTION_SAVE);

  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Save", Gtk::RESPONSE_OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("Glight show");
  filter->add_pattern("*.gshow");
  filter->add_mime_type("text/gshow+xml");
  dialog.add_filter(filter);

  int result = dialog.run();
  if(result == Gtk::RESPONSE_OK)
	{
		Glib::ustring filename(dialog.get_filename());
		if(filename.find('.') == Glib::ustring::npos)
			filename += ".gshow";

		std::lock_guard<std::mutex> lock(_management->Mutex());
		Writer writer(*_management);
		writer.SetGUIState(_state);
		writer.Write(filename);
	}
}

void ShowWindow::onMIQuitClicked()
{
	hide();
}

void ShowWindow::onMIDryModeClicked()
{
	Folder& activeFolder = _objectListFrame->SelectedFolder();
	std::string path = activeFolder.FullPath();
	bool enterDryMode = _miDryMode.get_active();
	if(enterDryMode && _backgroundManagement == nullptr)
	{
		// Switch from real mode to dry mode
		_backgroundManagement = std::move(_management);
		_management = _backgroundManagement->MakeDryMode();
		_management->Run();
		_visualizationWindow->SetDryMode(_management.get());
		changeManagement(_management.get(), false);
	}
	else if(!enterDryMode && _backgroundManagement != nullptr)
	{
		// Switch from dry mode to real mode
		_management->SwapDevices(*_backgroundManagement);
		_visualizationWindow->MakeDryModeReal();
		_backgroundManagement.reset();
	}
	_miCancelDryMode.set_sensitive(enterDryMode);
	_miOpen.set_sensitive(!enterDryMode);
	_miSave.set_sensitive(!enterDryMode);
	FolderObject& folder = _management->GetObjectFromPath(path);
	_objectListFrame->OpenFolder(static_cast<Folder&>(folder));
}

void ShowWindow::onMICancelDryModeClicked()
{
	if(_miDryMode.get_active() && _backgroundManagement != nullptr)
	{
		Gtk::MessageDialog dialog(*this, "Are you sure you want to cancel dry mode?",
			false, Gtk::MESSAGE_QUESTION,
			Gtk::BUTTONS_OK_CANCEL);
		dialog.set_secondary_text("All changes made after entering dry mode will be lost");
		int result = dialog.run();
		if(result == Gtk::RESPONSE_OK)
		{
			Folder& activeFolder = _objectListFrame->SelectedFolder();
			std::string path = activeFolder.FullPath();
			
			std::swap(_backgroundManagement, _management);
			_visualizationWindow->SetRealMode();
			changeManagement(_management.get(), true);
			_backgroundManagement.reset();
			_miCancelDryMode.set_sensitive(false);
			_miDryMode.set_active(false);
			
			FolderObject& folder = _management->GetObjectFromPath(path);
			_objectListFrame->OpenFolder(static_cast<Folder&>(folder));
		}
	}
}

void ShowWindow::changeManagement(Management* newManagement, bool moveControlSliders)
{
	_state.ChangeManagement(*newManagement);
	_signalChangeManagement(*newManagement);
	for(std::unique_ptr<FaderWindow>& cw :_faderWindows)
		cw->ChangeManagement(*newManagement, moveControlSliders);
	_sceneFrame->ChangeManagement(*newManagement);
	EmitUpdate();
}

void ShowWindow::onControlWindowHidden(FaderWindow* window)
{
	for(std::vector<std::unique_ptr<FaderWindow>>::iterator i=_faderWindows.begin(); i!=_faderWindows.end(); ++i)
	{
		if(i->get() == window)
		{
			_faderWindows.erase(i);
			break;
		}
	}
}

size_t ShowWindow::nextControlKeyRow() const
{
	size_t index = 0;
	while(index < std::numeric_limits<size_t>::max()) {
		bool found = false;
		for(const std::unique_ptr<FaderWindow>& cw : _faderWindows)
		{
			if(index == cw->KeyRowIndex())
			{
				++index;
				found = true;
				break;
			}
		}
		if(!found)
			return index;
	}
	throw std::runtime_error("Error in nextControlKeyRow()");
}

void ShowWindow::onMIDesignWizardClicked()
{
	std::string path = _objectListFrame->SelectedFolder().FullPath();
	if(!_designWizard || !_designWizard->is_visible())
		_designWizard.reset(new DesignWizard(*_management, *this, path));
	_designWizard->SetDestinationPath(path);
	_designWizard->present();
}
