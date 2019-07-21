#ifndef CHASE_PROPERTIES_WINDOW_H
#define CHASE_PROPERTIES_WINDOW_H

#include "propertieswindow.h"

#include "components/durationinput.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/separator.h>
#include <gtkmm/window.h>

/**
	@author Andre Offringa
*/
class ChasePropertiesWindow : public PropertiesWindow {
public:
	ChasePropertiesWindow(class Chase& chase, class Management& management, class EventTransmitter& eventHub);
	~ChasePropertiesWindow();

	class FolderObject& GetObject() final override;
	class Chase& GetChase() { return *_chase; }
	
private:
	void loadChaseInfo(class Chase& chase);
	void onTriggerTypeChanged();
	void onTriggerSpeedChanged(double newValue);
	void onTransitionSpeedChanged(double newValue);
	void onTransitionTypeChanged();
	void onSyncCountChanged();
	void onBeatSpeedChanged();
	
	void onChangeManagement(class Management& management)
	{
		_management = &management;
	}
	void onUpdateControllables();
	void onToTimeSequenceClicked();
	
	Gtk::VBox _box;
	Gtk::Grid _grid;
	
	Gtk::RadioButton _delayTriggerCheckButton;
	DurationInput _triggerDuration;
	DurationInput _transitionDuration;
	Gtk::HBox _transitionTypeBox;
	Gtk::Label _transitionTypeLabel;
	Gtk::RadioButton _transitionNoneRB, _transitionFadeRB, _transitionFadeThroughBlackRB, _transitionErraticRB;
	Gtk::HSeparator _transitionSep;
	
	Gtk::RadioButton _synchronizedTriggerCheckButton;
	Gtk::Label _synchronizationsLabel;
	Gtk::HScale _synchronizationsCount;
	Gtk::HSeparator _synchronizedSep;
	
	Gtk::RadioButton _beatTriggerCheckButton;
	Gtk::Label _beatSpeedLabel;
	Gtk::HScale _beatSpeed;
	
	Gtk::HButtonBox _buttonBox;
	Gtk::Button _toTimeSequenceButton, _closeButton;
	
	Chase* _chase;
	Management* _management;
	EventTransmitter& _eventHub;
};

#endif
