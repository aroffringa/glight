#ifndef TIME_SEQUENCE_PROPERTIES_WINDOW_H
#define TIME_SEQUENCE_PROPERTIES_WINDOW_H

#include "avoidrecursion.h"
#include "propertieswindow.h"

#include "components/objectbrowser.h"

#include "../libtheatre/timesequence.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/frame.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/separator.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

/**
	@author Andre Offringa
*/
class TimeSequencePropertiesWindow : public PropertiesWindow {
public:
	TimeSequencePropertiesWindow(TimeSequence& timeSequence, class Management& management, class ShowWindow& parentWindow);
	~TimeSequencePropertiesWindow();

	class FolderObject& GetObject() final override;
	class TimeSequence& GetTimeSequence() { return *_timeSequence; }
	
private:
	void onSelectedStepChanged();
	void load();
	void fillStepsList();
	void loadStep(const TimeSequence::Step& step);
	void onAddStep();
	void onRemoveStep();
	void onSustainChanged();
	void onRepeatChanged();
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
	void setStepSensitive(bool sensitive);
	TimeSequence::Step* selectedStep();
	void selectStep(size_t index);
	
	Gtk::HBox _topBox;
	ObjectBrowser _objectBrowser;
	
	Gtk::VBox _buttonBox;
	Gtk::Button _addStepButton;
	Gtk::Button _removeStepButton;
	
	Gtk::Grid _grid;
	Gtk::TreeView _stepsView;
	Glib::RefPtr<Gtk::ListStore> _stepsStore;
	struct StepsListColumns : public Gtk::TreeModelColumnRecord
	{
		StepsListColumns()
			{ add(_title); add(_trigger); add(_step); }
	
		Gtk::TreeModelColumn<Glib::ustring> _title;
		Gtk::TreeModelColumn<Glib::ustring> _trigger;
		Gtk::TreeModelColumn<size_t> _step;
	} _stepsListColumns;
	Gtk::ScrolledWindow _stepsScrolledWindow;
	
	Gtk::CheckButton _sustainCB;
	Gtk::CheckButton _maxRepeatCB;
	Gtk::HScale _maxRepeatCount;
	
	Gtk::RadioButton _delayTriggerCheckButton;
	Gtk::HScale _triggerSpeed;
	
	Gtk::RadioButton _synchronizedTriggerCheckButton;
	Gtk::HScale _synchronizationsCount;
	
	Gtk::RadioButton _beatTriggerCheckButton;
	Gtk::HScale _beatSpeed;
	
	Gtk::Label _transitionSpeedLabel;
	Gtk::HScale _transitionSpeed;
	Gtk::HBox _transitionTypeBox;
	Gtk::Label _transitionTypeLabel;
	Gtk::RadioButton _transitionNoneRB, _transitionFadeRB, _transitionFadeThroughBlackRB, _transitionErraticRB;
	
	AvoidRecursion _recursionLock;
	
	TimeSequence* _timeSequence;
	Management* _management;
	ShowWindow& _parentWindow;
};

#endif

