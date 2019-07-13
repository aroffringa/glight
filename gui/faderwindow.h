#ifndef CONTROLWINDOW_H
#define CONTROLWINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/grid.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scale.h>
#include <gtkmm/window.h>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include "recursionlock.h"

/**
	@author Andre Offringa
*/
class FaderWindow  : public Gtk::Window {
	public:
		/**
		 * Construct a control window with a new, empty fader setup.
		 */
		FaderWindow(class ShowWindow* showWindow, class Management& management, size_t keyRowIndex);
		
		/**
		 * Construct a control window and load an existing fader setup.
		 */
		FaderWindow(class ShowWindow* showWindow, class Management& management, size_t keyRowIndex, class FaderSetupState* state);
		
		~FaderWindow();

		void Update();
		bool HandleKeyDown(char key);
		bool HandleKeyUp(char key);
		bool IsAssigned(class PresetValue* presetValue);
		size_t KeyRowIndex() const { return _keyRowIndex; }
		
		/**
		 * Associates all faders to a different management instance.
		 * 
		 * Faders will be assigned to the presets of the new management instance, by looking
		 * up the preset by Id. This method is e.g. used for switching between dry and real mode.
		 * Faders that are assigned to a preset with an Id that does not correspond to a preset
		 * in the new instance are unassigned.
		 */
		void ChangeManagement(class Management& management, bool moveSliders);
		
	private:
		void initializeWidgets();
		
		void onAddButtonClicked() { addControl(); }
		void onRemoveButtonClicked();
		void onAssignButtonClicked();
		void onAssignChasesButtonClicked();
		void onSoloButtonToggled();
		void onNameButtonClicked();
		void onNewFaderSetupButtonClicked();
		void onControlValueChanged(double newValue, class ControlWidget* widget);
		void onControlAssigned(double newValue, size_t widgetIndex);
		bool onResize(GdkEventConfigure *event);
		double mapSliderToSpeed(int sliderVal);
		void onChangeUpSpeed();
		void onChangeDownSpeed();
		bool onTimeout() { updateValues(); return true; }
		
		void addControl();
		
		void onFaderSetupChanged();
		void updateFaderSetupList();
		void loadState();
		void updateValues();

		class Management* _management;
		size_t _keyRowIndex;

		Gtk::VBox _vBox;
		Gtk::Label _faderSetupLabel;
		Gtk::ComboBox _faderSetup;
		Glib::RefPtr<Gtk::ListStore> _faderSetupList;
		Gtk::HBox _hBoxUpper, _hBox2;
		Gtk::Grid _controlGrid;
		Gtk::Button _nameButton, _newFaderSetupButton;
		Gtk::CheckButton _soloCheckButton;
		Gtk::HScale _fadeUpSpeed, _fadeDownSpeed;
		Gtk::Button _addButton, _assignButton, _assignChasesButton, _removeButton;
		Gtk::VButtonBox _buttonBox;

		std::vector<std::unique_ptr<class ControlWidget>> _controls;
		class ShowWindow* _showWindow;
		class FaderSetupState* _state;
		RecursionLock _delayUpdates;
		sigc::connection _faderSetupChangeConnection, _timeoutConnection;
		static const char _keyRowsUpper[3][10], _keyRowsLower[3][10];
		
		boost::posix_time::ptime _lastUpdateTime;
		
		class FaderSetupColumns : public Gtk::TreeModel::ColumnRecord
		{
		public:
			FaderSetupColumns()
			{ add(_obj); add(_name);}

			Gtk::TreeModelColumn<class FaderSetupState*> _obj;
			Gtk::TreeModelColumn<Glib::ustring> _name;
		} _faderSetupColumns;
};

#endif
