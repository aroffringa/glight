#include "chasepropertieswindow.h"
#include "eventtransmitter.h"

#include "../theatre/chase.h"
#include "../theatre/folder.h"
#include "../theatre/functiontype.h"
#include "../theatre/management.h"
#include "../theatre/timesequence.h"

ChasePropertiesWindow::ChasePropertiesWindow(class Chase& chase, Management &management, EventTransmitter &eventHub) :
	PropertiesWindow(),
	_delayTriggerCheckButton("Delayed trigger"),
	_triggerDuration("Trigger duration:", 500.0),
	_transitionDuration("Transition duration:", 500.0),
	
	_synchronizedTriggerCheckButton("Synchronized"),
	_synchronizationsLabel("Nr. of synchronizations:"),
	_synchronizationsCount(1.0, 100.0, 1.0),
		
	_beatTriggerCheckButton("Trigger by beat"),
	_beatSpeedLabel("Beats per trigger :"),
	_beatSpeed(0.25, 4.0, 0.25),
	
	_toTimeSequenceButton("Convert to time sequence"),
	_closeButton("Close"),
	
	_chase(&chase),
	_management(&management),
	_eventHub(eventHub)
{
	_eventHub.SignalChangeManagement().connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onChangeManagement));
	_eventHub.SignalUpdateControllables().connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onUpdateControllables));
	
	set_title(chase.Name() + " — glight");
	
	Gtk::RadioButtonGroup group;
	_grid.attach(_delayTriggerCheckButton, 0, 0, 1, 3);
	_delayTriggerCheckButton.set_group(group);
	_delayTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onTriggerTypeChanged));
	_grid.attach(_triggerDuration, 1, 0, 2, 1);
	_triggerDuration.SignalValueChanged().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onTriggerSpeedChanged));

	_grid.attach(_transitionDuration, 1, 1, 2, 1);
	_transitionDuration.SignalValueChanged().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onTransitionSpeedChanged));
	
	_transitionTypeBox.SignalChanged().
		connect(sigc::mem_fun(*this, &ChasePropertiesWindow::onTransitionTypeChanged));
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
	_box.pack_start(_grid);
	
	_toTimeSequenceButton.signal_clicked().connect([&](){ onToTimeSequenceClicked(); });
	_buttonBox.pack_start(_toTimeSequenceButton, true, false, 5);
	
	_closeButton.set_image_from_icon_name("window-close");
	_closeButton.signal_clicked().connect([&](){ hide(); });
	_buttonBox.pack_end(_closeButton, true, false, 5);
	
	_box.pack_end(_buttonBox, true, true, 2);
	
	add(_box);
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

void ChasePropertiesWindow::onTriggerSpeedChanged(double newValue)
{
	double curTime = _management->GetOffsetTimeInMS();
	double transitionValue = _transitionDuration.Value();
	if(newValue == 0.0 && transitionValue == 0.0)
	{
		_transitionDuration.SetValue(40.0);
		std::lock_guard<std::mutex> lock(_management->Mutex());
		_chase->ShiftDelayTrigger(newValue, 40.0, curTime);
	}
	else {
		std::lock_guard<std::mutex> lock(_management->Mutex());
		_chase->ShiftDelayTrigger(newValue, transitionValue, curTime);
	}
}

void ChasePropertiesWindow::onTransitionSpeedChanged(double newValue)
{
	double curTime = _management->GetOffsetTimeInMS();
	double triggerValue = _triggerDuration.Value();
	if(triggerValue == 0.0 && newValue == 0.0)
	{
		_triggerDuration.SetValue(40.0);
		std::lock_guard<std::mutex> lock(_management->Mutex());
		_chase->ShiftDelayTrigger(40.0, newValue, curTime);
	}
	else {
		std::lock_guard<std::mutex> lock(_management->Mutex());
		_chase->ShiftDelayTrigger(triggerValue, newValue, curTime);
	}
}

void ChasePropertiesWindow::onTransitionTypeChanged(enum Transition::Type type)
{
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
	_triggerDuration.SetValue(triggerSpeed);
	_transitionDuration.SetValue(transitionSpeed);
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
	_transitionTypeBox.Set(transitionType);
}

void ChasePropertiesWindow::onUpdateControllables()
{
	if(_management->Contains(*_chase))
		loadChaseInfo(*_chase);
	else 
		hide();
}

void ChasePropertiesWindow::onToTimeSequenceClicked()
{
	TimeSequence& tSequence = _management->AddTimeSequence();
	tSequence.SetRepeatCount(0);
	size_t index = 0;
	for(const std::pair<Controllable*, size_t>& item : _chase->Sequence().List())
	{
		tSequence.AddStep(*item.first, item.second);
		TimeSequence::Step& step = tSequence.GetStep(index);
		if(_chase->Trigger().Type() == Trigger::DelayTriggered)
			step.transition = _chase->Transition();
		else
			step.transition.SetLengthInMs(0);
		step.trigger = _chase->Trigger();
		++index;
	}
	Folder& folder = _chase->Parent();
	std::string name = _chase->Name();
	PresetValue* preset = _management->GetPresetValue(*_chase, 0);
	preset->Reconnect(tSequence, 0);
	_management->RemoveControllable(*_chase);
	tSequence.SetName(name);
	folder.Add(tSequence);
	//_management->AddPreset(tSequence, 0);
	_eventHub.EmitUpdate();
}
