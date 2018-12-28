#ifndef CONTROLWINDOW_H
#define CONTROLWINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
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
		void onControlValueChanged(double newValue, class ControlWidget* widget);
		
		void addControl();

		class Management &_management;
		size_t _keyRowIndex;

		Gtk::HBox _hBox;
		Gtk::CheckButton _soloCheckButton;
		Gtk::Button _addButton, _assignButton, _assignChasesButton, _removeButton;
		Gtk::VButtonBox _buttonBox;

		std::vector<class ControlWidget *> _controls;
		class ShowWindow* _showWindow;
		static const char _keyRowsUpper[3][10], _keyRowsLower[3][10];
};

#endif
