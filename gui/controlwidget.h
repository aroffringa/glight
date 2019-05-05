#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/eventbox.h>

#include <memory>

#include "components/controllableselectmenu.h"

/**
	@author Andre Offringa
*/
class ControlWidget : public Gtk::VBox {
	public:
		ControlWidget(class Management& management, char key);
		~ControlWidget();

		void Update();
		void Toggle();
		void FullOn();
		void FullOff();
		void Assign(class PresetValue* item, bool moveFader);
		void Unassign() { Assign(nullptr, false); }
		PresetValue* Preset() const { return _preset; }
		
		sigc::signal<void, double>& SignalValueChange() { return _signalValueChange; }
		sigc::signal<void>& SignalAssigned() { return _signalAssigned; }
		
		void Limit(double value)
		{
			if(_scale.get_value() > value)
				_scale.set_value(value);
		}
		
		static double MAX_SCALE_VALUE();
		
		void SetFadeUpSpeed(double fadePerSecond) { _fadeUpSpeed = fadePerSecond; }
		void SetFadeDownSpeed(double fadePerSecond) { _fadeDownSpeed = fadePerSecond; }
		
		void UpdateValue(double timePassed);
		void ChangeManagement(class Management& management, bool moveSliders);
		
		Gtk::Widget& NameLabel() { return _eventBox; }
		
	private:
		void writeValue();
		void onScaleChange();
		void onOnButtonClicked();
		bool onNameLabelClicked(GdkEventButton* event);
		void onControllableSelected(PresetValue *item);
		bool onFlashButtonPressed(GdkEventButton* event);
		bool onFlashButtonReleased(GdkEventButton* event);

		Gtk::VScale _scale;
		Gtk::Button _flashButton;
		Gtk::CheckButton _onCheckButton;
		Gtk::EventBox _eventBox;
		Gtk::Label _nameLabel;
		ControllableSelectMenu _menu;

		class Management* _management;
		class PresetValue* _preset;
		double _fadeUpSpeed, _fadeDownSpeed;
		unsigned _fadingValue, _targetValue;

		bool _holdUpdates;
		
		sigc::signal<void, double> _signalValueChange;
		sigc::signal<void> _signalAssigned;
};

#endif
