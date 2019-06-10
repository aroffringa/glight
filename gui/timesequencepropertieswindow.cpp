
#include "timesequencepropertieswindow.h"
#include "showwindow.h"

#include "../theatre/management.h"

#include <gtkmm/messagedialog.h>

TimeSequencePropertiesWindow::TimeSequencePropertiesWindow(class TimeSequence& timeSequence, Management &management, ShowWindow &parentWindow) :
	PropertiesWindow(),
	_objectBrowser(management, parentWindow),
	
	_sustainCB("Sustain"),
	_maxRepeatCB("Max repeats:"),
	_maxRepeatCount(1.0, 100.0, 1.0),
	
	_delayTriggerCheckButton("Delayed trigger, in ms:"),
	_triggerSpeed(1.0, 10000.0, 100.0),
	
	_synchronizedTriggerCheckButton("Synchronized, count:"),
	_synchronizationsCount(1.0, 100.0, 1.0),
		
	_beatTriggerCheckButton("Trigger by beat, count:"),
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
	
	_addStepButton.set_image_from_icon_name("go-next");
	_addStepButton.signal_clicked().connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onAddStep));
	_buttonBox.pack_start(_addStepButton, false, false, 4);
	
	_removeStepButton.set_image_from_icon_name("go-previous");
	_removeStepButton.signal_clicked().connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onRemoveStep));
	_buttonBox.pack_start(_removeStepButton, false, false, 4);
	
	_buttonBox.set_valign(Gtk::ALIGN_CENTER);
	_topBox.pack_start(_buttonBox);
	
	_sustainCB.signal_clicked().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onSustainChanged));
	_grid.attach(_sustainCB, 0, 0, 2, 1);
	_maxRepeatCB.signal_clicked().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onRepeatChanged));
	_grid.attach(_maxRepeatCB, 0, 1, 1, 1);
	_maxRepeatCount.signal_value_changed().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onRepeatChanged));
	_grid.attach(_maxRepeatCount, 1, 1, 1, 1);
	
	_stepsStore = Gtk::ListStore::create(_stepsListColumns);

	_stepsView.set_model(_stepsStore);
	_stepsView.append_column("Controllable", _stepsListColumns._title);
	_stepsView.append_column("Trigger", _stepsListColumns._trigger);
	fillStepsList();
	_stepsView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onSelectedStepChanged));
	_stepsScrolledWindow.add(_stepsView);
	
	_stepsScrolledWindow.set_size_request(200, 200);
	_stepsScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_grid.attach(_stepsScrolledWindow, 0, 2, 2, 1);
	
	Gtk::RadioButtonGroup group;
	_grid.attach(_delayTriggerCheckButton, 0, 3, 1, 1);
	_delayTriggerCheckButton.set_group(group);
	_delayTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTriggerTypeChanged));
	_grid.attach(_triggerSpeed, 1, 3, 1, 1);
	_triggerSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTriggerSpeedChanged));
	
	_grid.attach(_synchronizedTriggerCheckButton, 0, 4, 1, 1);
	_synchronizedTriggerCheckButton.set_group(group);
	_synchronizedTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTriggerTypeChanged));
	_grid.attach(_synchronizationsCount, 1, 4, 1, 1);
	_synchronizationsCount.set_value(1.0);
	_synchronizationsCount.signal_value_changed().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onSyncCountChanged));

	_grid.attach(_beatTriggerCheckButton, 0, 5, 1, 1);
	_beatTriggerCheckButton.set_group(group);
	_beatTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onTriggerTypeChanged));
	_grid.attach(_beatSpeed, 1, 5, 1, 1);
	_beatSpeed.set_hexpand(true);
	_beatSpeed.set_value(1.0);
	_beatSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &TimeSequencePropertiesWindow::onBeatSpeedChanged));
	
	_transitionSpeedLabel.set_halign(Gtk::ALIGN_END);
	_grid.attach(_transitionSpeedLabel, 0, 6, 1, 1);
	_grid.attach(_transitionSpeed, 1, 6, 1, 1);
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
	
	_grid.attach(_transitionTypeBox, 0, 7, 2, 1);
	
	_grid.set_hexpand(true);
	_topBox.add(_grid);
	add(_topBox);
	show_all_children();
	
	load();
	onSelectedStepChanged();
}

TimeSequencePropertiesWindow::~TimeSequencePropertiesWindow()
{
}

void TimeSequencePropertiesWindow::load()
{
	_sustainCB.set_active(_timeSequence->Sustain());
	if(_timeSequence->RepeatCount() == 0)
		_maxRepeatCB.set_active(false);
	else {
		_maxRepeatCount.set_value(_timeSequence->RepeatCount());
		_maxRepeatCB.set_active(true);
	}
	fillStepsList();
}

FolderObject& TimeSequencePropertiesWindow::GetObject()
{
	return GetTimeSequence();
}

TimeSequence::Step* TimeSequencePropertiesWindow::selectedStep()
{
	Gtk::TreeModel::iterator selIter = _stepsView.get_selection()->get_selected();
	if(selIter)
	{
		size_t index = (*selIter)[_stepsListColumns._step];
		return &_timeSequence->GetStep(index);
	}
	else {
		return nullptr;
	}
}

void TimeSequencePropertiesWindow::selectStep(size_t index)
{
	_stepsView.get_selection()->select(_stepsStore->children()[index]);
}

void TimeSequencePropertiesWindow::fillStepsList()
{
	AvoidRecursion::Token token(_recursionLock);
	Gtk::TreeModel::iterator iter = _stepsView.get_selection()->get_selected();
	bool hasSelection = false;
	size_t index = 0;
	if(iter)
	{
		hasSelection = true;
		index = (*iter)[_stepsListColumns._step];
	}
	_stepsStore->clear();
	for(size_t i=0; i!=_timeSequence->Size(); ++i)
	{
		Gtk::TreeModel::iterator iter = _stepsStore->append();
		Gtk::TreeModel::Row row = *iter;
		row[_stepsListColumns._title] = _timeSequence->Sequence().List()[i].first->Name();
		row[_stepsListColumns._trigger] = _timeSequence->GetStep(i).trigger.ToString();
		row[_stepsListColumns._step] = i;
		if(hasSelection && i == index)
		{
			_stepsView.get_selection()->select(row);
		}
	}
	token.Release();
	if(hasSelection && !_stepsView.get_selection()->get_selected())
		onSelectedStepChanged();
}

void TimeSequencePropertiesWindow::onAddStep()
{
	Controllable* object = dynamic_cast<Controllable*>(_objectBrowser.SelectedObject());
	if(object)
	{
		std::unique_lock<std::mutex> lock(_management->Mutex());
		_timeSequence->AddStep(*object, 0);
		if(_management->HasCycle())
		{
			_timeSequence->RemoveStep(_timeSequence->Size()-1);
			lock.unlock();
			Gtk::MessageDialog dialog(
				"Can not add this object to the time sequence: this would create a cycle in the connections.",
				false,
				Gtk::MESSAGE_ERROR
			);
			dialog.run();
		}
		else {
			lock.unlock();
			fillStepsList();
			selectStep(_timeSequence->Size()-1);
		}
	}
}

void TimeSequencePropertiesWindow::onRemoveStep()
{
	Controllable* object = dynamic_cast<Controllable*>(_objectBrowser.SelectedObject());
	if(object)
	{
		Gtk::TreeModel::iterator selIter = _stepsView.get_selection()->get_selected();
		if(selIter)
		{
			size_t index = (*selIter)[_stepsListColumns._step];
			std::unique_lock<std::mutex> lock(_management->Mutex());
			_timeSequence->RemoveStep(index);
			lock.unlock();
			
			fillStepsList();
		}
	}
}

void TimeSequencePropertiesWindow::onSustainChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	_timeSequence->SetSustain(_sustainCB.get_active());
}

void TimeSequencePropertiesWindow::onRepeatChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	if(_maxRepeatCB.get_active())
		_timeSequence->SetRepeatCount(_maxRepeatCount.get_value());
	else
		_timeSequence->SetRepeatCount(0);
}

void TimeSequencePropertiesWindow::onTriggerTypeChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	TimeSequence::Step* step = selectedStep();
	if(step)
	{
		if(_delayTriggerCheckButton.get_active())
			step->trigger.SetType(Trigger::DelayTriggered);
		else if(_synchronizedTriggerCheckButton.get_active())
			step->trigger.SetType(Trigger::SyncTriggered);
		else
			step->trigger.SetType(Trigger::BeatTriggered);
		fillStepsList();
	}
}

void TimeSequencePropertiesWindow::onTriggerSpeedChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	TimeSequence::Step* step = selectedStep();
	if(step)
	{
		step->trigger.SetDelayInMs(_triggerSpeed.get_value());
		fillStepsList();
	}
}

void TimeSequencePropertiesWindow::onTransitionSpeedChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	TimeSequence::Step* step = selectedStep();
	if(step)
	{
		step->transition.SetLengthInMs(_transitionSpeed.get_value());
		fillStepsList();
	}
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
	TimeSequence::Step* step = selectedStep();
	if(step)
	{
		step->transition.SetType(type);
		fillStepsList();
	}
}

void TimeSequencePropertiesWindow::onSyncCountChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	TimeSequence::Step* step = selectedStep();
	if(step)
	{
		step->trigger.SetDelayInSyncs(_synchronizationsCount.get_value());
		fillStepsList();
	}
}

void TimeSequencePropertiesWindow::onBeatSpeedChanged()
{
	std::lock_guard<std::mutex> lock(_management->Mutex());
	TimeSequence::Step* step = selectedStep();
	if(step)
	{
		step->trigger.SetDelayInBeats(_beatSpeed.get_value());
		fillStepsList();
	}
}

void TimeSequencePropertiesWindow::onSelectedStepChanged()
{
	if(_recursionLock.IsFirst())
	{
		TimeSequence::Step* step = selectedStep();
		if(step)
		{
			loadStep(*step);
			setStepSensitive(true);
		}
		else {
			setStepSensitive(false);
		}
	}
}

void TimeSequencePropertiesWindow::loadStep(const TimeSequence::Step& step)
{
	std::unique_lock<std::mutex> lock(_management->Mutex());
	enum Trigger::Type triggerType = step.trigger.Type();
	enum Transition::Type transitionType = step.transition.Type();
	double triggerSpeed = step.trigger.DelayInMs();
	double transitionSpeed = step.transition.LengthInMs();
	double beatSpeed = step.trigger.DelayInBeats();
	double syncSpeed = step.trigger.DelayInSyncs();
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

void TimeSequencePropertiesWindow::setStepSensitive(bool sensitive)
{
	_removeStepButton.set_sensitive(sensitive);
	
	_delayTriggerCheckButton.set_sensitive(sensitive);
	_triggerSpeed.set_sensitive(sensitive);
	_synchronizedTriggerCheckButton.set_sensitive(sensitive);
	_synchronizationsCount.set_sensitive(sensitive);
	_beatTriggerCheckButton.set_sensitive(sensitive);
	_beatSpeed.set_sensitive(sensitive);
	_transitionErraticRB.set_sensitive(sensitive);
	_transitionSpeed.set_sensitive(sensitive);
	_transitionNoneRB.set_sensitive(sensitive);
	_transitionFadeRB.set_sensitive(sensitive);
	_transitionFadeThroughBlackRB.set_sensitive(sensitive);
	_transitionErraticRB.set_sensitive(sensitive);
}

void TimeSequencePropertiesWindow::onUpdateControllables()
{
	if(_management->Contains(*_timeSequence))
		load();
	else 
		hide();
}