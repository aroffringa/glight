
#include "timesequencepropertieswindow.h"
#include "showwindow.h"

#include "../libtheatre/management.h"

TimeSequencePropertiesWindow::TimeSequencePropertiesWindow(class TimeSequence& timeSequence, Management &management, ShowWindow &parentWindow) :
	PropertiesWindow(),
	_objectBrowser(management, parentWindow),
	
	_delayTriggerCheckButton("Delayed trigger"),
	_triggerSpeedLabel("Trigger speed (ms) :"),
	_triggerSpeed(1.0, 10000.0, 100.0),
	
	_synchronizedTriggerCheckButton("Synchronized"),
	_synchronizationsLabel("Nr. of synchronizations:"),
	_synchronizationsCount(1.0, 100.0, 1.0),
		
	_beatTriggerCheckButton("Trigger by beat"),
	_beatSpeedLabel("Beats per trigger :"),
	_beatSpeed(0.25, 4.0, 0.25),
	
	_transitionSpeedLabel("Transition speed (ms) :"),
	_transitionSpeed(0.0, 10000.0, 100.0),
	_transitionTypeLabel("Type:"),
	_transitionNoneRB("None"),
	_transitionFadeRB("Fade"),
	_transitionFadeThroughBlackRB("Through black"),
	_transitionErraticRB("Erratic"),
	
	_timeSequence(&timeSequence),
	_management(&management),
	_parentWindow(parentWindow)
{
	parentWindow.SignalChangeManagement().connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onChangeManagement));
	parentWindow.SignalUpdateControllables().connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onUpdateControllables));
	
	set_title("glight - " + timeSequence.Name());
	
	_topBox.pack_start(_objectBrowser);
	_objectBrowser.set_size_request(200, 200);
	
	_addButton.set_image_from_icon_name("go-next");
	_addButton.set_valign(Gtk::ALIGN_CENTER);
	_topBox.pack_start(_addButton, false, false, 4);
	
	_stepsStore = Gtk::ListStore::create(_stepsListColumns);

	_stepsView.set_model(_stepsStore);
	_stepsView.append_column("Controllable", _stepsListColumns._title);
	_stepsView.append_column("Trigger", _stepsListColumns._trigger);
	fillStepsList();
	_stepsScrolledWindow.set_size_request(200, 200);
	_stepsScrolledWindow.add(_stepsView);

	_stepsScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_grid.attach(_stepsScrolledWindow, 0, 0, 3, 1);
	
	Gtk::RadioButtonGroup group;
	_grid.attach(_delayTriggerCheckButton, 0, 1, 1, 1);
	_delayTriggerCheckButton.set_group(group);
	_delayTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTriggerTypeChanged));
	_grid.attach(_triggerSpeedLabel, 1, 1, 1, 1);
	_triggerSpeedLabel.set_halign(Gtk::ALIGN_END);
	_grid.attach(_triggerSpeed, 2, 1, 1, 1);
	_triggerSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTriggerSpeedChanged));
	_grid.attach(_triggerSep, 0, 2, 3, 1);
	
	_grid.attach(_synchronizedTriggerCheckButton, 0, 3, 1, 1);
	_synchronizedTriggerCheckButton.set_group(group);
	_synchronizedTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTriggerTypeChanged));
	_synchronizationsLabel.set_halign(Gtk::ALIGN_END);
	_grid.attach(_synchronizationsLabel, 1, 3, 1, 1);
	_grid.attach(_synchronizationsCount, 2, 3, 1, 1);
	_synchronizationsCount.set_value(1.0);
	_synchronizationsCount.signal_value_changed().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onSyncCountChanged));
	_grid.attach(_synchronizedSep, 0, 4, 3, 1);

	_grid.attach(_beatTriggerCheckButton, 0, 5, 1, 1);
	_beatTriggerCheckButton.set_group(group);
	_beatTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTriggerTypeChanged));
	_beatSpeedLabel.set_halign(Gtk::ALIGN_END);
	_grid.attach(_beatSpeedLabel, 1, 5, 1, 1);
	_grid.attach(_beatSpeed, 2, 5, 1, 1);
	_beatSpeed.set_hexpand(true);
	_beatSpeed.set_value(1.0);
	_beatSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onBeatSpeedChanged));
	
	_transitionSpeedLabel.set_halign(Gtk::ALIGN_END);
	_grid.attach(_transitionSpeedLabel, 0, 6, 1, 1);
	_grid.attach(_transitionSpeed, 1, 6, 2, 1);
	_transitionSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTransitionSpeedChanged));
	
	_transitionTypeBox.pack_start(_transitionTypeLabel);
		
	Gtk::RadioButtonGroup transTypeGroup;
	_transitionNoneRB.set_group(transTypeGroup);
	_transitionNoneRB.signal_clicked().connect(
		sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionNoneRB);
	
	_transitionFadeRB.set_group(transTypeGroup);
	_transitionFadeRB.signal_clicked().connect(
		sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionFadeRB);
	
	_transitionFadeThroughBlackRB.set_group(transTypeGroup);
	_transitionFadeThroughBlackRB.signal_clicked().connect(
		sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionFadeThroughBlackRB);
	
	_transitionErraticRB.set_group(transTypeGroup);
	_transitionErraticRB.signal_clicked().connect(
		sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionErraticRB);
	
	_grid.attach(_transitionTypeBox, 0, 7, 3, 1);
	
	_grid.set_hexpand(true);
	_topBox.add(_grid);
	add(_topBox);
	show_all_children();
	
	load(timeSequence);
}

TimeSequencePropertiesWindow::~TimeSequencePropertiesWindow()
{
}

FolderObject& TimeSequencePropertiesWindow::GetObject()
{
	return GetTimeSequence();
}

TimeSequence::Step& TimeSequencePropertiesWindow::selectedStep()
{
	
}

void TimeSequencePropertiesWindow::fillStepsList()
{
}

void TimeSequencePropertiesWindow::onTriggerTypeChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	TimeSequence::Step& step = selectedStep();
	if(_delayTriggerCheckButton.get_active())
		step.trigger.SetType(Trigger::DelayTriggered);
	else if(_synchronizedTriggerCheckButton.get_active())
		step.trigger.SetType(Trigger::SyncTriggered);
	else
		step.trigger.SetType(Trigger::BeatTriggered);
}

void TimeSequencePropertiesWindow::onTriggerSpeedChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	TimeSequence::Step& step = selectedStep();
	step.trigger.SetDelayInMs(_triggerSpeed.get_value());
}

void TimeSequencePropertiesWindow::onTransitionSpeedChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	TimeSequence::Step& step = selectedStep();
	step.transition.SetLengthInMs(_transitionSpeed.get_value());
}

void TimeSequencePropertiesWindow::onTransitionTypeChanged()
{
	enum Transition::Type type;
	if(_transitionNoneRB.get_active())
		type = Transition::None;
	else if(_transitionFadeRB.get_active())
		type = Transition::Fade;
	else if(_transitionFadeThroughBlackRB.get_active())
		type = Transition::FadeThroughBlack;
	else //if(_transitionErraticRB.get_active())
		type = Transition::Erratic;
	std::lock_guard<std::mutex> lock(_management->Mutex());
	selectedStep().transition.SetType(type);
}

void TimeSequencePropertiesWindow::onSyncCountChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	selectedStep().trigger.SetDelayInSyncs(_synchronizationsCount.get_value());
}

void TimeSequencePropertiesWindow::onBeatSpeedChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	selectedStep().trigger.SetDelayInBeats(_beatSpeed.get_value());
}

void TimeSequencePropertiesWindow::load(const TimeSequence& timeSequence)
{
}

void TimeSequencePropertiesWindow::loadStep(const TimeSequence::Step& step)
{
	std::unique_lock<std::mutex> lock(_management->Mutex());
	enum Trigger::Type triggerType = step.trigger.Type();
	enum Transition::Type transitionType = step.transition.Type();
	double triggerSpeed = step.trigger.DelayInMs();
	double transitionSpeed = step.transition.LengthInMs();
	double beatSpeed = step.trigger.DelayInBeats();
	lock.unlock();
	_triggerSpeed.set_value(triggerSpeed);
	_transitionSpeed.set_value(transitionSpeed);
	_beatSpeed.set_value(beatSpeed);
	if(triggerType == Trigger::DelayTriggered)
		_delayTriggerCheckButton.set_active(true);
	else
		_beatTriggerCheckButton.set_active(true);
	switch(transitionType)
	{
		case Transition::None:
			_transitionNoneRB.set_active();
			break;
		case Transition::Fade:
			_transitionFadeRB.set_active();
			break;
		case Transition::FadeThroughBlack:
			_transitionFadeRB.set_active();
			break;
		case Transition::Erratic:
			_transitionErraticRB.set_active();
			break;
	}
}

void TimeSequencePropertiesWindow::onUpdateControllables()
{
	if(_management->Contains(*_timeSequence))
		load(*_timeSequence);
	else 
		hide();
}
