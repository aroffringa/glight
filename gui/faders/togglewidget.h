#ifndef FADERS_TOGGLE_WIDGET_H
#define FADERS_TOGGLE_WIDGET_H

#include "controlwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>

class ToggleWidget : public ControlWidget 
{
public:
	ToggleWidget(class Management& management, class EventTransmitter& eventHub, char key);
	~ToggleWidget();
	
	virtual void Toggle() final override;
	virtual void FullOn() final override;
	virtual void FullOff() final override;
	virtual void Assign(class PresetValue* item, bool moveFader) final override;
	virtual class PresetValue* Preset() const final override
	{
		return _preset;
	}
	
	virtual void Limit(double value) final override;
	virtual void ChangeManagement(class Management& management, bool moveSliders) final override;
	
private:
	Gtk::HBox _box;
	Gtk::VBox _flashButtonBox;
	Gtk::Button _flashButton;
	Gtk::CheckButton _onCheckButton;
	Gtk::EventBox _eventBox;
	Gtk::Label _nameLabel;

	sigc::connection _updateConnection;
	class Management* _management;
	class EventTransmitter& _eventHub;
	class PresetValue* _preset;

	bool _holdUpdates;
	
	void onUpdate();
	void onOnButtonClicked();
	bool onNameLabelClicked(GdkEventButton* event);
	bool onFlashButtonPressed(GdkEventButton* event);
	bool onFlashButtonReleased(GdkEventButton* event);
};

#endif
