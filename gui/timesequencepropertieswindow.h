#ifndef TIME_SEQUENCE_PROPERTIES_WINDOW_H
#define TIME_SEQUENCE_PROPERTIES_WINDOW_H

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
	void fillStepsList();
	void load(const TimeSequence& timeSequence);
	void loadStep(const TimeSequence::Step& step);
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
	
	TimeSequence::Step& selectedStep();
	
	Gtk::HBox _topBox;
	ObjectBrowser _objectBrowser;
	
	Gtk::Button _addButton;
	
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
	
	Gtk::RadioButton _delayTriggerCheckButton;
	Gtk::Label _triggerSpeedLabel;
	Gtk::HScale _triggerSpeed;
	Gtk::HSeparator _triggerSep;
	
	Gtk::RadioButton _synchronizedTriggerCheckButton;
	Gtk::Label _synchronizationsLabel;
	Gtk::HScale _synchronizationsCount;
	Gtk::HSeparator _synchronizedSep;
	
	Gtk::RadioButton _beatTriggerCheckButton;
	Gtk::Label _beatSpeedLabel;
	Gtk::HScale _beatSpeed;
	
	Gtk::Label _transitionSpeedLabel;
	Gtk::HScale _transitionSpeed;
	Gtk::HBox _transitionTypeBox;
	Gtk::Label _transitionTypeLabel;
	Gtk::RadioButton _transitionNoneRB, _transitionFadeRB, _transitionFadeThroughBlackRB, _transitionErraticRB;
	
	TimeSequence* _timeSequence;
	Management* _management;
	ShowWindow& _parentWindow;
};

#endif

