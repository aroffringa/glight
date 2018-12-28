#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/menu.h>

/**
	@author Andre Offringa
*/
class ControlWidget : public Gtk::VBox {
	public:
		ControlWidget(class Management &management, char key);
		~ControlWidget();

		void UpdateAfterPresetRemoval();
		void Toggle();
		void FullOn();
		void FullOff();
		void Assign(class PresetValue* item);
		void Unassign() { Assign(0); }
		PresetValue* Preset() const { return _preset; }
		
		sigc::signal<void,double>& SignalChange() { return _signalChange; }
		
		void Limit(double value)
		{
			if(_scale.get_value() > value)
				_scale.set_value(value);
		}
		
		static double MAX_SCALE_VALUE();
	private:
		void writeValue();
		void onScaleChange();
		void onOnButtonClicked();
		bool onNameLabelClicked(GdkEventButton* event);
		void onMenuItemClicked(class PresetValue *item);
		bool onFlashButtonPressed(GdkEventButton* event);
		bool onFlashButtonReleased(GdkEventButton* event);

		void DestroyPopupMenu();

		Gtk::VScale _scale;
		Gtk::Button _flashButton;
		Gtk::CheckButton _onCheckButton;
		Gtk::EventBox _eventBox;
		Gtk::Label _nameLabel;
		Gtk::Menu *_popupMenu, *_popupChaseMenu, *_popupPresetMenu, *_popupFunctionMenu;
		std::vector<Gtk::MenuItem *> _popupMenuItems;

		class Management &_management;
		class PresetValue *_preset;

		bool _holdUpdates;
		
		sigc::signal<void,double> _signalChange;
};

#endif
