#ifndef SHOWWINDOW_H
#define SHOWWINDOW_H

#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/window.h>

#include "guistate.h"

/**
	@author Andre Offringa
*/
class ShowWindow : public Gtk::Window {
	public:
		ShowWindow(std::unique_ptr<class DmxDevice> dmxDevice);
		~ShowWindow();
		
		bool IsAssignedToControl(class PresetValue* presetValue);

		void EmitUpdate();
		void EmitUpdateAfterPresetRemoval();
		void EmitUpdateAfterAddPreset();
		
		GUIState& State() { return _state; }
		
	private:
		void onProgramWindowButtonClicked();
		void onControlWindowButtonClicked()
		{
			addControlWindow();
		}
		void onConfigurationWindowButtonClicked();
		void onVisualizationWindowButtonClicked();
		void addControlWindow();

		bool onKeyDown(GdkEventKey *event);
		bool onKeyUp(GdkEventKey *event);
		
		void createMenu();

		void onMINewClicked();
		void onMIOpenClicked();
		void onMISaveClicked();
		void onMIQuitClicked();
		
		void onControlWindowHidden(class ControlWindow* window);
		
		size_t nextControlKeyRow() const;

		Gtk::VBox _box;
		Gtk::ToggleButton _programWindowButton;
		Gtk::Button _controlWindowButton;
		Gtk::ToggleButton _configurationWindowButton;
		Gtk::ToggleButton _visualizationWindowButton;
		Gtk::HButtonBox _windowButtonsBox;

		class ProgramWindow *_programWindow;
		std::vector<class ControlWindow *> _controlWindows;
		class ConfigurationWindow *_configurationWindow;
		class VisualizationWindow *_visualizationWindow;

		class Management *_management;
		class PresetCollection *_preset;

		class SceneFrame *_sceneFrame;

		GUIState _state;
		
		Gtk::Menu _menuFile;
		
		Gtk::MenuItem _miFile;
		Gtk::ImageMenuItem _miNew, _miOpen, _miSave, _miQuit;
		
		Gtk::MenuBar _menuBar;
};

#endif
