#ifndef CHASE_PROPERTIES_WINDOW_H
#define CHASE_PROPERTIES_WINDOW_H

#include "propertieswindow.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
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
	ChasePropertiesWindow(class Chase& chase, class Management& management, class ShowWindow& parentWindow);
	~ChasePropertiesWindow();

	class FolderObject& GetObject() final override;
	class Chase& GetChase() { return *_chase; }
	
private:
	void loadChaseInfo(class Chase& chase);
	void onTriggerTypeChanged();
	void onTriggerSpeedChanged();
	void onTransitionSpeedChanged();
	void onTransitionTypeChanged();
	void onSyncCountChanged();
	void onBeatSpeedChanged();
	
	void onChangeManagement(class Management& management)
	{
		_management = &management;
	}
	void onUpdateControllables();
	
	Gtk::Frame _frame;
	Gtk::Grid _grid;
	
	Gtk::RadioButton _delayTriggerCheckButton;
	Gtk::Label _triggerSpeedLabel;
	Gtk::HScale _triggerSpeed;
	Gtk::Label _transitionSpeedLabel;
	Gtk::HScale _transitionSpeed;
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
	
	Chase* _chase;
	Management* _management;
	ShowWindow& _parentWindow;
};

#endif
