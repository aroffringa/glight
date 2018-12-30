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
	_programWindowButton("Programming"),
	_controlWindowButton("Controls"),
	_configurationWindowButton("Config"),
	_visualizationWindowButton("Visualization"),
	_miFile("_File", true),
	_miNew(Gtk::Stock::NEW),
	_miOpen(Gtk::Stock::OPEN),
	_miSave(Gtk::Stock::SAVE_AS),
	_miQuit(Gtk::Stock::QUIT)
{
	set_title("Glight - show");

	Theatre *theatre = new Theatre();
	_management = new Management(*theatre);
	_management->AddDevice(std::move(device));

	_management->Run();

	_programWindow = new ProgramWindow(*_management, *this);

	_programWindow->signal_key_press_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyDown));
	_programWindow->signal_key_release_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyUp));
	_programWindow->show();
	
	addControlWindow();

	_configurationWindow = new ConfigurationWindow(*_management);
	_configurationWindow->signal_key_press_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyDown));
	_configurationWindow->signal_key_release_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyUp));

	_visualizationWindow = new VisualizationWindow(*_management);

	createMenu();
	
	_programWindowButton.set_active(true);
	_programWindowButton.signal_clicked().connect(sigc::mem_fun(*this, &ShowWindow::onProgramWindowButtonClicked));
	_windowButtonsBox.pack_start(_programWindowButton);
	_programWindowButton.show();

	_controlWindowButton.signal_clicked().connect(sigc::mem_fun(*this, &ShowWindow::onControlWindowButtonClicked));
	_windowButtonsBox.pack_start(_controlWindowButton);
	_controlWindowButton.show();

	_configurationWindowButton.set_active(false);
	_configurationWindowButton.signal_clicked().connect(sigc::mem_fun(*this, &ShowWindow::onConfigurationWindowButtonClicked));
	_windowButtonsBox.pack_start(_configurationWindowButton);
	_configurationWindowButton.show();

	_visualizationWindowButton.set_active(false);
	_visualizationWindowButton.signal_clicked().connect(sigc::mem_fun(*this, &ShowWindow::onVisualizationWindowButtonClicked));
	_windowButtonsBox.pack_start(_visualizationWindowButton);
	_visualizationWindowButton.show();

	_box.pack_start(_windowButtonsBox, false, false);
	_windowButtonsBox.show();

	_sceneFrame = new SceneFrame(*_management);
	_box.pack_start(*_sceneFrame, true, true);
	_sceneFrame->show();
	
	add(_box);
	_box.show_all();
		
	signal_key_press_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyDown));
	signal_key_release_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyUp));
}


ShowWindow::~ShowWindow()
{
	_menuFile.detach();
	
	delete _sceneFrame;
	delete _visualizationWindow;
	delete _configurationWindow;

	_controlWindows.clear();

	delete _programWindow;
	Theatre *theatre = &_management->Theatre();
	delete _management;
	delete theatre;
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
	bool show = _programWindowButton.get_active();
	if(show)
		_programWindow->show();
	else
		_programWindow->hide();
}

void ShowWindow::addControlWindow()
{
	FaderSetupState* inactiveState = nullptr;
	for(std::unique_ptr<FaderSetupState>& setup : _state.FaderSetups())
	{
		if(!setup->isActive)
		{
			inactiveState = setup.get();
			break;
		}
	}
	if(inactiveState == nullptr)
		_controlWindows.emplace_back(new ControlWindow(this, *_management, nextControlKeyRow()));
	else
		_controlWindows.emplace_back(new ControlWindow(this, *_management, nextControlKeyRow(), inactiveState));
	ControlWindow *newWindow = _controlWindows.back().get();
	newWindow->signal_key_press_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyDown));
	newWindow->signal_key_release_event().connect(sigc::mem_fun(*this, &ShowWindow::onKeyUp));
	newWindow->signal_hide().connect(sigc::bind(sigc::mem_fun(*this, &ShowWindow::onControlWindowHidden), newWindow));
	newWindow->show();
}

void ShowWindow::onConfigurationWindowButtonClicked()
{
	bool show = _configurationWindowButton.get_active();
	if(show)
		_configurationWindow->show();
	else
		_configurationWindow->hide();
}

void ShowWindow::onVisualizationWindowButtonClicked()
{
	bool show = _visualizationWindowButton.get_active();
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
		Reader reader(*_management);
		reader.Read(dialog.get_filename());

		if(_management->Show().Scenes().size() != 0)
			_sceneFrame->SetSelectedScene(*_management->Show().Scenes()[0]);
		else
			_sceneFrame->SetNoSelectedScene();

		lock.unlock();
	
		EmitUpdate();
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

bool ShowWindow::IsAssignedToControl(PresetValue* presetValue) const
{
	for(const std::unique_ptr<ControlWindow>& cw : _controlWindows)
	{
		if(cw->IsAssigned(presetValue))
			return true;
	}
	return false;
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
