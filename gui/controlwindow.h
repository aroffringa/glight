#ifndef CONTROLWINDOW_H
#define CONTROLWINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scale.h>
#include <gtkmm/window.h>

/**
	@author Andre Offringa
*/
class ControlWindow  : public Gtk::Window {
	public:
		ControlWindow(class ShowWindow* showWindow, class Management& management, size_t keyRowIndex);
		~ControlWindow();

		void Update() { UpdateAfterPresetRemoval(); }
		void UpdateAfterPresetRemoval();
		bool HandleKeyDown(char key);
		bool HandleKeyUp(char key);
		bool IsAssigned(class PresetValue* presetValue);
		size_t KeyRowIndex() const { return _keyRowIndex; }
		
	private:
		void onAddButtonClicked() { addControl(); }
		void onRemoveButtonClicked();
		void onAssignButtonClicked();
		void onAssignChasesButtonClicked();
		void onSoloButtonToggled();
		void onNameButtonClicked();
		void onControlValueChanged(double newValue, class ControlWidget* widget);
		
		void addControl();
		
		void updateFaderSetupList();

		class Management &_management;
		size_t _keyRowIndex;

		Gtk::VBox _vBox;
		Gtk::Label _faderSetupLabel;
		Gtk::ComboBox _faderSetup;
		Glib::RefPtr<Gtk::ListStore> _faderSetupList;
		Gtk::HBox _hBoxUpper, _hBox2;
		Gtk::Button _nameButton;
		Gtk::CheckButton _soloCheckButton;
		Gtk::Button _addButton, _assignButton, _assignChasesButton, _removeButton;
		Gtk::VButtonBox _buttonBox;

		std::vector<std::unique_ptr<class ControlWidget>> _controls;
		class ShowWindow* _showWindow;
		class FaderSetupState* _state;
		static const char _keyRowsUpper[3][10], _keyRowsLower[3][10];
		
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
