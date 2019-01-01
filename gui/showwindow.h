#ifndef SHOWWINDOW_H
#define SHOWWINDOW_H

#include <vector>

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkmenuitem.h>
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
		
		/**
		 * If stateOrNull is nullptr, the first inactive state is selected, or
		 * if no states are inactive, a new state is created.
		 */
		void addControlWindow(FaderSetupState* stateOrNull = nullptr);

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

		std::unique_ptr<class ProgramWindow> _programWindow;
		std::vector<std::unique_ptr<class ControlWindow>> _controlWindows;
		std::unique_ptr<class ConfigurationWindow> _configurationWindow;
		std::unique_ptr<class VisualizationWindow> _visualizationWindow;

		std::unique_ptr<class Management> _management;
		class PresetCollection *_preset;

		std::unique_ptr<class SceneFrame> _sceneFrame;

		GUIState _state;
		
		Gtk::Menu _menuFile, _menuWindow;
		
		Gtk::MenuItem _miFile, _miWindow;
		Gtk::ImageMenuItem _miNew, _miOpen, _miSave, _miQuit;
		Gtk::CheckMenuItem _miProgrammingWindow, _miConfigWindow;
		Gtk::MenuItem _miNewControlWindow;
		Gtk::CheckMenuItem _miVisualizationWindow;
		
		Gtk::MenuBar _menuBar;
};

#endif
