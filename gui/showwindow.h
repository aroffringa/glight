#ifndef SHOWWINDOW_H
#define SHOWWINDOW_H

#include <vector>

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

#include "eventtransmitter.h"
#include "guistate.h"

/**
	@author Andre Offringa
*/
class ShowWindow : public Gtk::Window, public EventTransmitter {
public:
	ShowWindow(std::unique_ptr<class DmxDevice> dmxDevice);
	~ShowWindow();
	
	void EmitUpdate() final override;
	
	GUIState& State() { return _state; }
	
	sigc::signal<void(Management&)>& SignalChangeManagement() final override
	{ return _signalChangeManagement; }
	sigc::signal<void()>& SignalUpdateControllables() final override
	{ return _signalUpdateControllables; }
	
	Management& GetManagement() const { return *_management; }
	
	void OpenFile(const std::string& filename);
	
private:
	void onControlWindowButtonClicked()
	{
		addFaderWindow();
	}
	void onConfigurationWindowButtonClicked();
	void onVisualizationWindowButtonClicked();
	
	/**
		* If stateOrNull is nullptr, the first inactive state is selected, or
		* if no states are inactive, a new state is created.
		*/
	void addFaderWindow(FaderSetupState* stateOrNull = nullptr);

	bool onKeyDown(GdkEventKey *event);
	bool onKeyUp(GdkEventKey *event);
	bool onDelete(GdkEventAny *event);
	
	void createMenu();

	void onMINewClicked();
	void onMIOpenClicked();
	void onMISaveClicked();
	void onMIQuitClicked();
	
	void onMIDryModeClicked();
	void onMICancelDryModeClicked();
	void onMIDesignWizardClicked();
	
	void onControlWindowHidden(class FaderWindow* window);
	
	void changeManagement(class Management* newManagement, bool moveControlSliders);
	
	size_t nextControlKeyRow() const;

	Gtk::VBox _box;

	std::vector<std::unique_ptr<class FaderWindow>> _faderWindows;
	std::unique_ptr<class ConfigurationWindow> _configurationWindow;
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
	
	sigc::signal<void(Management&)> _signalChangeManagement;
	sigc::signal<void()> _signalUpdateControllables;
	
	Gtk::Menu _menuFile, _menuDesign, _menuWindow;
	
	Gtk::MenuItem _miFile, _miDesign, _miWindow;
	Gtk::ImageMenuItem _miNew, _miOpen, _miSave, _miQuit;
	Gtk::CheckMenuItem _miDryMode;
	Gtk::MenuItem _miCancelDryMode;
	Gtk::SeparatorMenuItem _miDesignSep1;
	Gtk::MenuItem _miDesignWizard;
	Gtk::CheckMenuItem _miConfigWindow;
	Gtk::MenuItem _miNewControlWindow;
	Gtk::CheckMenuItem _miVisualizationWindow;
	
	Gtk::MenuBar _menuBar;
	Gtk::Notebook _notebook;
};

#endif
