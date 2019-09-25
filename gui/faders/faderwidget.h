#ifndef FADER_WIDGET_H
#define FADER_WIDGET_H

#include "controlwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/eventbox.h>

/**
	@author Andre Offringa
*/
class FaderWidget : public ControlWidget {
public:
	FaderWidget(class Management& management, class EventTransmitter& eventHub, char key);
	~FaderWidget();

	void Toggle() final override;
	void FullOn() final override;
	void FullOff() final override;
	void Assign(class PresetValue* item, bool moveFader) final override;
	void MoveSlider() final override;
	PresetValue* Preset() const final override { return _preset; }
	
	void Limit(double value) final override
	{
		if(_scale.get_value() > value)
			_scale.set_value(value);
	}
	
	void ChangeManagement(class Management& management, bool moveSliders) final override;
	
	Gtk::Widget& NameLabel() { return _eventBox; }
	
private:
	void onUpdate();
	void onScaleChange();
	void onOnButtonClicked();
	bool onNameLabelClicked(GdkEventButton* event);
	bool onFlashButtonPressed(GdkEventButton* event);
	bool onFlashButtonReleased(GdkEventButton* event);

	Gtk::VBox _box;
	Gtk::VScale _scale;
	Gtk::Button _flashButton;
	Gtk::CheckButton _onCheckButton;
	Gtk::EventBox _eventBox;
	Gtk::Label _nameLabel;

	sigc::connection _updateConnection;
	class Management* _management;
	class EventTransmitter& _eventHub;
	class PresetValue* _preset;

	bool _holdUpdates;
};

#endif
