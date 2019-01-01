#include "showwindow.h"

#include <gtkmm/stock.h>
#include <gtkmm/filechooserdialog.h>

#include "configurationwindow.h"
#include "controlwindow.h"
#include "programwindow.h"
#include "sceneframe.h"
#include "visualizationwindow.h"

#include "../libtheatre/dmxdevice.h"
#include "../libtheatre/management.h"
#include "../libtheatre/theatre.h"
#include "../libtheatre/fixture.h"
#include "../libtheatre/fixturefunctioncontrol.h"
#include "../libtheatre/presetcollection.h"
#include "../libtheatre/show.h"

#include "../reader.h"
#include "../writer.h"

ShowWindow::ShowWindow(std::unique_ptr<DmxDevice> device) :
	_miFile("_File", true),
	_miOptions("_Options", true),
	_miWindow("_Window", true),
	_miNew(Gtk::Stock::NEW),
	_miOpen(Gtk::Stock::OPEN),
	_miSave(Gtk::Stock::SAVE_AS),
	_miQuit(Gtk::Stock::QUIT),
	_miDryMode("Dry mode"),
	_miProgrammingWindow("Programming"),
	_miConfigWindow("Config"),
	_miNewControlWindow("New faders window"),
	_miVisualizationWindow("Visualization")
{
	set_title("Glight - show");

	_management.reset(new Management());
	_management->AddDevice(std::move(device));

	_management->Run();

	_programWindow.reset(new ProgramWindow(*_management, *this));

	_programWindow->signal_key_press_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyDown));
	_programWindow->signal_key_release_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyUp));
	_programWindow->show();
	
	addControlWindow();

	_configurationWindow.reset(new ConfigurationWindow(*_management));
	_configurationWindow->signal_key_press_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyDown));
	_configurationWindow->signal_key_release_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyUp));

	_visualizationWindow.reset(new VisualizationWindow(_management.get()));

	createMenu();
	
	_sceneFrame.reset(new SceneFrame(*_management));
	_box.pack_start(*_sceneFrame, true, true);
	
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

	_controlWindows.clear();

	_programWindow.reset();
	_management.reset();
}

void ShowWindow::EmitUpdate()
{
	_programWindow->Update();
	for(std::unique_ptr<ControlWindow>& cw : _controlWindows)
		cw->Update();
	_configurationWindow->Update();
	_sceneFrame->Update();
}

void ShowWindow::EmitUpdateAfterPresetRemoval()
{
	_programWindow->UpdateAfterPresetRemoval();
	for(std::unique_ptr<ControlWindow>& cw : _controlWindows)
		cw->UpdateAfterPresetRemoval();
}

void ShowWindow::EmitUpdateAfterAddPreset()
{
	_sceneFrame->Update();
}

void ShowWindow::onProgramWindowButtonClicked()
{
	bool show = _miProgrammingWindow.get_active();
	if(show)
		_programWindow->show();
	else
		_programWindow->hide();
}

void ShowWindow::addControlWindow(FaderSetupState* stateOrNull)
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
	if(stateOrNull == nullptr)
		_controlWindows.emplace_back(new ControlWindow(this, *_management, nextControlKeyRow()));
	else
		_controlWindows.emplace_back(new ControlWindow(this, *_management, nextControlKeyRow(), stateOrNull));
	ControlWindow *newWindow = _controlWindows.back().get();
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
	for(std::unique_ptr<ControlWindow>& cw : _controlWindows)
		if(!handled)
			handled = cw->HandleKeyDown(event->keyval);
	return handled;
}

bool ShowWindow::onKeyUp(GdkEventKey *event)
{
	bool handled = false;
	for(std::unique_ptr<ControlWindow>& cw : _controlWindows)
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
	
	_menuOptions.set_title("_Options");
	
	_miDryMode.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onMIDryModeClicked));
	_menuOptions.append(_miDryMode);
		
	_miOptions.set_submenu(_menuOptions);
	_menuBar.append(_miOptions);
	
	_menuWindow.set_title("_Window");
	
	_miProgrammingWindow.signal_activate().connect(sigc::mem_fun(*this, &ShowWindow::onProgramWindowButtonClicked));
	_miProgrammingWindow.set_active(true);
	_menuWindow.append(_miProgrammingWindow);
	
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
	{
		std::unique_lock<std::mutex> lock(_management->Mutex());
		_management->Clear();
		_controlWindows.clear();
		_state.Clear();
		Reader reader(*_management);
		reader.SetGUIState(_state);
		reader.Read(dialog.get_filename());

		if(_management->Show().Scenes().size() != 0)
			_sceneFrame->SetSelectedScene(*_management->Show().Scenes()[0]);
		else
			_sceneFrame->SetNoSelectedScene();
		
		lock.unlock();
	
		EmitUpdate();
		
		if(_state.Empty())
		{
			std::cout << "File did not contain GUI state info: will start with default faders.\n";
			addControlWindow();
		}
		else {
			for(const std::unique_ptr<FaderSetupState>& state : _state.FaderSetups())
			{
				if(state->isActive)
				{
					// Currently it is not displayed, so to avoid the control window doing the
					// wrong thing, isActive is set to false and will be set to true by the control window.
					state->isActive = false;
					addControlWindow(state.get());
				}
			}
		}
	}
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
	if(_miDryMode.get_active() && _backgroundManagement == nullptr)
	{
		// Switch from real mode to dry mode
		_backgroundManagement = std::move(_management);
		_management = _backgroundManagement->MakeDryMode();
		_visualizationWindow->SetDryMode(_management.get());
		EmitUpdate();
	}
	else {
		// Switch from dry mode to real mode
		_visualizationWindow->SetRealMode();
	}
}

void ShowWindow::onControlWindowHidden(ControlWindow* window)
{
	for(std::vector<std::unique_ptr<ControlWindow>>::iterator i=_controlWindows.begin(); i!=_controlWindows.end(); ++i)
	{
		if(i->get() == window)
		{
			_controlWindows.erase(i);
			break;
		}
	}
}

size_t ShowWindow::nextControlKeyRow() const
{
	size_t index = 0;
	while(index < std::numeric_limits<size_t>::max()) {
		bool found = false;
		for(const std::unique_ptr<ControlWindow>& cw : _controlWindows)
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
