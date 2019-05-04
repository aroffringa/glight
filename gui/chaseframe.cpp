#include "chaseframe.h"
#include "showwindow.h"

#include "../libtheatre/chase.h"
#include "../libtheatre/management.h"

ChaseFrame::ChaseFrame(Management &management, ShowWindow &parentWindow) :
	_upperFrame("All chases"),
	_chaseList(management, parentWindow),
	_deleteChaseButton("Delete"),
	_bottomFrame("Selected chase"),
	
	_delayTriggerCheckButton("Delayed trigger"),
	_triggerSpeedLabel("Trigger speed (ms) :"),
	_triggerSpeed(1.0, 10000.0, 100.0),
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
	_management(&management),
	_parentWindow(parentWindow)
{
	initUpperPanel();
	initLowerPanel();
}

ChaseFrame::~ChaseFrame()
{
}

void ChaseFrame::initUpperPanel()
{
	_deleteChaseButton.signal_clicked().connect(sigc::mem_fun(*this, &ChaseFrame::onDeleteChaseClicked));
	_upperButtonBox.pack_start(_deleteChaseButton);
	
	_upperBox.pack_start(_upperButtonBox, false, false, 5);

	_chaseList.SetDisplayType(ObjectTree::OnlyChases);
	_chaseList.SignalSelectionChange().connect(
		sigc::mem_fun(*this, &ChaseFrame::onSelectedChaseChanged));
	_upperBox.pack_start(_chaseList);

	_upperFrame.add(_upperBox);

	pack1(_upperFrame);
	_upperFrame.show_all();
}

void ChaseFrame::initLowerPanel()
{
	Gtk::RadioButtonGroup group;
	_bottomGrid.attach(_delayTriggerCheckButton, 0, 0, 1, 3);
	_delayTriggerCheckButton.set_group(group);
	_delayTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ChaseFrame::onTriggerTypeChanged));
	_bottomGrid.attach(_triggerSpeedLabel, 1, 0);
	_triggerSpeedLabel.set_halign(Gtk::ALIGN_END);
	_bottomGrid.attach(_triggerSpeed, 2, 0);
	_triggerSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChaseFrame::onTriggerSpeedChanged));

	_transitionSpeedLabel.set_halign(Gtk::ALIGN_END);
	_bottomGrid.attach(_transitionSpeedLabel, 1, 1);
	_bottomGrid.attach(_transitionSpeed, 2, 1);
	_transitionSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChaseFrame::onTransitionSpeedChanged));
	
	_transitionTypeBox.pack_start(_transitionTypeLabel);
		
	Gtk::RadioButtonGroup transTypeGroup;
	_transitionNoneRB.set_group(transTypeGroup);
	_transitionNoneRB.signal_clicked().connect(
		sigc::mem_fun(*this, &ChaseFrame::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionNoneRB);
	
	_transitionFadeRB.set_group(transTypeGroup);
	_transitionFadeRB.signal_clicked().connect(
		sigc::mem_fun(*this, &ChaseFrame::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionFadeRB);
	
	_transitionFadeThroughBlackRB.set_group(transTypeGroup);
	_transitionFadeThroughBlackRB.signal_clicked().connect(
		sigc::mem_fun(*this, &ChaseFrame::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionFadeThroughBlackRB);
	
	_transitionErraticRB.set_group(transTypeGroup);
	_transitionErraticRB.signal_clicked().connect(
		sigc::mem_fun(*this, &ChaseFrame::onTransitionTypeChanged));
	_transitionTypeBox.pack_start(_transitionErraticRB);
	
	_bottomGrid.attach(_transitionTypeBox, 1, 2, 2, 1);
	_bottomGrid.attach(_transitionSep, 0, 3, 3, 1);
	
	_bottomGrid.attach(_synchronizedTriggerCheckButton, 0, 4);
	_synchronizedTriggerCheckButton.set_group(group);
	_synchronizedTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ChaseFrame::onTriggerTypeChanged));
	_synchronizationsLabel.set_halign(Gtk::ALIGN_END);
	_bottomGrid.attach(_synchronizationsLabel, 1, 4);
	_bottomGrid.attach(_synchronizationsCount, 2, 4);
	_synchronizationsCount.set_value(1.0);
	_synchronizationsCount.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChaseFrame::onSyncCountChanged));
	_bottomGrid.attach(_synchronizedSep, 0, 5, 3, 1);

	_bottomGrid.attach(_beatTriggerCheckButton, 0, 6);
	_beatTriggerCheckButton.set_group(group);
	_beatTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ChaseFrame::onTriggerTypeChanged));
	_beatSpeedLabel.set_halign(Gtk::ALIGN_END);
	_bottomGrid.attach(_beatSpeedLabel, 1, 6);
	_bottomGrid.attach(_beatSpeed, 2, 6);
	_beatSpeed.set_hexpand(true);
	_beatSpeed.set_value(1.0);
	_beatSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChaseFrame::onBeatSpeedChanged));
	
	_bottomGrid.set_hexpand(true);
	_bottomFrame.add(_bottomGrid);

	_bottomFrame.set_sensitive(false);
	_bottomFrame.show_all();
	pack2(_bottomFrame);
}

Chase* ChaseFrame::getSelectedChase()
{
	return dynamic_cast<Chase*>(_chaseList.SelectedObject());
}

void ChaseFrame::onTriggerTypeChanged()
{
	Chase* chase = getSelectedChase();
	if(chase)
	{
		std::lock_guard<std::mutex> lock(_management->Mutex());
		if(_delayTriggerCheckButton.get_active())
			chase->Trigger().SetType(Trigger::DelayTriggered);
		else if(_synchronizedTriggerCheckButton.get_active())
			chase->Trigger().SetType(Trigger::SyncTriggered);
		else
			chase->Trigger().SetType(Trigger::BeatTriggered);
	}
}

void ChaseFrame::onTriggerSpeedChanged()
{
	Chase* chase = getSelectedChase();
	if(chase)
	{
		std::lock_guard<std::mutex> lock(_management->Mutex());
		chase->Trigger().SetDelayInMs(_triggerSpeed.get_value());
	}
}

void ChaseFrame::onTransitionSpeedChanged()
{
	Chase* chase = getSelectedChase();
	if(chase)
	{
		std::lock_guard<std::mutex> lock(_management->Mutex());
		chase->Transition().SetLengthInMs(_transitionSpeed.get_value());
	}
}

void ChaseFrame::onTransitionTypeChanged()
{
	Chase* chase = getSelectedChase();
	if(chase)
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
		chase->Transition().SetType(type);
	}
}

void ChaseFrame::onSyncCountChanged()
{
	Chase* chase = getSelectedChase();
	if(chase)
	{
		std::lock_guard<std::mutex> lock(_management->Mutex());
		chase->Trigger().SetDelayInSyncs(_synchronizationsCount.get_value());
	}
}

void ChaseFrame::onBeatSpeedChanged()
{
	Chase* chase = getSelectedChase();
	if(chase)
	{
		std::lock_guard<std::mutex> lock(_management->Mutex());
		chase->Trigger().SetDelayInBeats(_beatSpeed.get_value());
	}
}

void ChaseFrame::onSelectedChaseChanged()
{
	Chase* chase = getSelectedChase();
	if(chase)
	{
		_bottomFrame.set_sensitive(true);
		std::unique_lock<std::mutex> lock(_management->Mutex());
		enum Trigger::Type triggerType = chase->Trigger().Type();
		enum Transition::Type transitionType = chase->Transition().Type();
		double triggerSpeed = chase->Trigger().DelayInMs();
		double transitionSpeed = chase->Transition().LengthInMs();
		double beatSpeed = chase->Trigger().DelayInBeats();
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
	else
	{
		_bottomFrame.set_sensitive(false);
	}
}

void ChaseFrame::onDeleteChaseClicked()
{
	Chase* chase = getSelectedChase();
	if(chase)
	{
		_management->RemoveControllable(*chase);
		_parentWindow.EmitUpdate();
	}
}

void ChaseFrame::Select(const Chase& chase)
{
	_chaseList.SelectObject(chase);
}
