#include "chasepropertieswindow.h"
#include "showwindow.h"

#include "../theatre/chase.h"
#include "../theatre/functiontype.h"
#include "../theatre/management.h"

ChasePropertiesWindow::ChasePropertiesWindow(class Chase& chase, Management &management, ShowWindow &parentWindow) :
	PropertiesWindow(),
	_frame("Properties of " + chase.Name()),
	_delayTriggerCheckButton("Delayed trigger"),
	_triggerSpeedLabel("Trigger speed (ms) :"),
	_triggerSpeed(0.0, 10000.0, 100.0),
	_transitionSpeedLabel("Transition speed (ms) :"),
	_transitionSpeed(0.0, 10000.0, 100.0),
	_transitionTypeLabel("Type:"),
	_transitionNoneRB("None"),
	_transitionFadeRB("Fade"),
	_transitionFadeThroughBlackRB("Through black"),
	_transitionErraticRB("Erratic"),
	
	_synchronizedTriggerCheckButton("Synchronized"),
	_synchronizationsLabel("Nr. of synchronizations:"),
	_synchronizationsCount(1.0, 100.0, 1.0),
		
	_beatTriggerCheckButton("Trigger by beat"),
	_beatSpeedLabel("Beats per trigger :"),
	_beatSpeed(0.25, 4.0, 0.25),
	_chase(&chase),
	_management(&management),
	_parentWindow(parentWindow)
{
	parentWindow.SignalChangeManagement().connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onChangeManagement));
	parentWindow.SignalUpdateControllables().connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onUpdateControllables));
	
	set_title("glight - " + chase.Name());
	
	Gtk::RadioButtonGroup group;
	_grid.attach(_delayTriggerCheckButton, 0, 0, 1, 3);
	_delayTriggerCheckButton.set_group(group);
	_delayTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onTriggerTypeChanged));
	_grid.attach(_triggerSpeedLabel, 1, 0, 1, 1);
	_triggerSpeedLabel.set_halign(Gtk::ALIGN_END);
	_grid.attach(_triggerSpeed, 2, 0, 1, 1);
	_triggerSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onTriggerSpeedChanged));

	_transitionSpeedLabel.set_halign(Gtk::ALIGN_END);
	_grid.attach(_transitionSpeedLabel, 1, 1, 1, 1);
	_grid.attach(_transitionSpeed, 2, 1, 1, 1);
	_transitionSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onTransitionSpeedChanged));
	
	_transitionTypeBox.pack_start(_transitionTypeLabel);
		
	Gtk::RadioButtonGroup transTypeGroup;
	_transitionNoneRB.set_group(transTypeGroup);
	_transitionNoneRB.signal_clicked().connect(
		sigc::mem_fun(*this, &ChasePropertiesWindow::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionNoneRB);
	
	_transitionFadeRB.set_group(transTypeGroup);
	_transitionFadeRB.signal_clicked().connect(
		sigc::mem_fun(*this, &ChasePropertiesWindow::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionFadeRB);
	
	_transitionFadeThroughBlackRB.set_group(transTypeGroup);
	_transitionFadeThroughBlackRB.signal_clicked().connect(
		sigc::mem_fun(*this, &ChasePropertiesWindow::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionFadeThroughBlackRB);
	
	_transitionErraticRB.set_group(transTypeGroup);
	_transitionErraticRB.signal_clicked().connect(
		sigc::mem_fun(*this, &ChasePropertiesWindow::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionErraticRB);
	
	_grid.attach(_transitionTypeBox, 1, 2, 2, 1);
	_grid.attach(_transitionSep, 0, 3, 3, 1);
	
	_grid.attach(_synchronizedTriggerCheckButton, 0, 4, 1, 1);
	_synchronizedTriggerCheckButton.set_group(group);
	_synchronizedTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onTriggerTypeChanged));
	_synchronizationsLabel.set_halign(Gtk::ALIGN_END);
	_grid.attach(_synchronizationsLabel, 1, 4, 1, 1);
	_grid.attach(_synchronizationsCount, 2, 4, 1, 1);
	_synchronizationsCount.set_value(1.0);
	_synchronizationsCount.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onSyncCountChanged));
	_grid.attach(_synchronizedSep, 0, 5, 3, 1);

	_grid.attach(_beatTriggerCheckButton, 0, 6, 1, 1);
	_beatTriggerCheckButton.set_group(group);
	_beatTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onTriggerTypeChanged));
	_beatSpeedLabel.set_halign(Gtk::ALIGN_END);
	_grid.attach(_beatSpeedLabel, 1, 6, 1, 1);
	_grid.attach(_beatSpeed, 2, 6, 1, 1);
	_beatSpeed.set_hexpand(true);
	_beatSpeed.set_value(1.0);
	_beatSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onBeatSpeedChanged));
	
	_grid.set_hexpand(true);
	_frame.add(_grid);
	add(_frame);
	show_all_children();
	
	loadChaseInfo(chase);
}

ChasePropertiesWindow::~ChasePropertiesWindow()
{
}

FolderObject& ChasePropertiesWindow::GetObject()
{
	return GetChase();
}

void ChasePropertiesWindow::onTriggerTypeChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	if(_delayTriggerCheckButton.get_active())
		_chase->Trigger().SetType(Trigger::DelayTriggered);
	else if(_synchronizedTriggerCheckButton.get_active())
		_chase->Trigger().SetType(Trigger::SyncTriggered);
	else
		_chase->Trigger().SetType(Trigger::BeatTriggered);
}

void ChasePropertiesWindow::onTriggerSpeedChanged()
{
	double curTime = _management->GetOffsetTimeInMS();
	if(_triggerSpeed.get_value() == 0.0 && _transitionSpeed.get_value() == 0.0)
	{
		_transitionSpeed.set_value(40.0);
		std::lock_guard<std::mutex> lock(_management->Mutex());
		_chase->ShiftDelayTrigger(_triggerSpeed.get_value(), 40.0, curTime);
	}
	else {
		std::lock_guard<std::mutex> lock(_management->Mutex());
		_chase->ShiftDelayTrigger(_triggerSpeed.get_value(), _transitionSpeed.get_value(), curTime);
	}
}

void ChasePropertiesWindow::onTransitionSpeedChanged()
{
	double curTime = _management->GetOffsetTimeInMS();
	if(_triggerSpeed.get_value() == 0.0 && _transitionSpeed.get_value() == 0.0)
	{
		_triggerSpeed.set_value(40.0);
		std::lock_guard<std::mutex> lock(_management->Mutex());
		_chase->ShiftDelayTrigger(40.0, _transitionSpeed.get_value(), curTime);
	}
	else {
		std::lock_guard<std::mutex> lock(_management->Mutex());
		_chase->ShiftDelayTrigger(_triggerSpeed.get_value(), _transitionSpeed.get_value(), curTime);
	}
}

void ChasePropertiesWindow::onTransitionTypeChanged()
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
	_chase->Transition().SetType(type);
}

void ChasePropertiesWindow::onSyncCountChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	_chase->Trigger().SetDelayInSyncs(_synchronizationsCount.get_value());
}

void ChasePropertiesWindow::onBeatSpeedChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	_chase->Trigger().SetDelayInBeats(_beatSpeed.get_value());
}

void ChasePropertiesWindow::loadChaseInfo(Chase& chase)
{
	std::unique_lock<std::mutex> lock(_management->Mutex());
	enum Trigger::Type triggerType = chase.Trigger().Type();
	enum Transition::Type transitionType = chase.Transition().Type();
	double triggerSpeed = chase.Trigger().DelayInMs();
	double transitionSpeed = chase.Transition().LengthInMs();
	double beatSpeed = chase.Trigger().DelayInBeats();
	double syncSpeed = chase.Trigger().DelayInSyncs();
	lock.unlock();
	_triggerSpeed.set_value(triggerSpeed);
	_transitionSpeed.set_value(transitionSpeed);
	_beatSpeed.set_value(beatSpeed);
	_synchronizationsCount.set_value(syncSpeed);
	switch(triggerType)
	{
	case Trigger::DelayTriggered:
		_delayTriggerCheckButton.set_active(true);
		break;
	case Trigger::SyncTriggered:
		_synchronizedTriggerCheckButton.set_active(true);
		break;
	case Trigger::BeatTriggered:
		_beatTriggerCheckButton.set_active(true);
		break;
	}
	switch(transitionType)
	{
		case Transition::None:
			_transitionNoneRB.set_active();
			break;
		case Transition::Fade:
			_transitionFadeRB.set_active();
			break;
		case Transition::FadeThroughBlack:
			_transitionFadeThroughBlackRB.set_active();
			break;
		case Transition::Erratic:
			_transitionErraticRB.set_active();
			break;
	}
}

void ChasePropertiesWindow::onUpdateControllables()
{
	if(_management->Contains(*_chase))
		loadChaseInfo(*_chase);
	else 
		hide();
}
