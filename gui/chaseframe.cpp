#include "chaseframe.h"
#include "showwindow.h"

#include "../libtheatre/chase.h"
#include "../libtheatre/management.h"

ChaseFrame::ChaseFrame(Management &management, ShowWindow &parentWindow) :
	_upperFrame("All chases"),
	_deleteChaseButton("Delete"),
	_bottomFrame("Selected chase"),
	
	_delayTriggerCheckButton("Trigger by delay"),
	_triggerSpeedLabel("Trigger speed (ms) :"),
	_triggerSpeed(1.0, 10000.0, 100.0),
	_transitionSpeedLabel("Transition speed (ms) :"),
	_transitionSpeed(0.0, 10000.0, 100.0),
	
	_synchronizedTriggerCheckButton("Synchronized trigger"),
	_synchronizationsLabel("Number of synchronizations"),
	_synchronizationsCount(1.0, 100.0, 1.0),
		
	_beatTriggerCheckButton("Trigger by beat"),
	_beatSpeedLabel("Beats per trigger :"),
	_beatSpeed(0.25, 4.0, 0.25),
	_management(&management), _parentWindow(parentWindow)
{
	initUpperPanel();

	initLowerPanel();
}

ChaseFrame::~ChaseFrame()
{
}

void ChaseFrame::fillChaseList()
{
	AvoidRecursion::Token token(_delayUpdates);
	_chaseListModel->clear();

	std::lock_guard<std::mutex> lock(_management->Mutex());
	const std::vector<std::unique_ptr<Controllable>>&
		controllables = _management->Controllables();
	for(const std::unique_ptr<Controllable>& c : controllables)
	{
		Chase* chase = dynamic_cast<Chase*>(c.get());
		if(chase != nullptr)
		{
			Gtk::TreeModel::iterator iter = _chaseListModel->append();
			Gtk::TreeModel::Row row = *iter;
			row[_chaseListColumns._title] = chase->Name();
			row[_chaseListColumns._chase] = chase;
		}
	}
}

void ChaseFrame::initUpperPanel()
{
	_deleteChaseButton.signal_clicked().connect(sigc::mem_fun(*this, &ChaseFrame::onDeleteChaseClicked));
	_upperButtonBox.pack_start(_deleteChaseButton);
	
	_upperBox.pack_start(_upperButtonBox, false, false, 5);

	_chaseListModel =
    Gtk::ListStore::create(_chaseListColumns);

	_chaseListView.set_model(_chaseListModel);
	_chaseListView.append_column("Chase", _chaseListColumns._title);
	_chaseListView.get_selection()->signal_changed().
		connect(sigc::mem_fun(*this, &ChaseFrame::onSelectedChaseChanged));
	_chaseScrolledWindow.add(_chaseListView);

	_chaseScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_upperBox.pack_start(_chaseScrolledWindow);

	_upperFrame.add(_upperBox);

	pack1(_upperFrame);
	_upperFrame.show_all();
}

void ChaseFrame::initLowerPanel()
{
	Gtk::RadioButtonGroup group;
	_bottomBox.pack_start(_delayTriggerCheckButton, false, false, 1);
	_delayTriggerCheckButton.set_group(group);
	_delayTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ChaseFrame::onTriggerTypeChanged));
	_bottomBox.pack_start(_triggerSpeedLabel, false, false, 1);
	_bottomBox.pack_start(_triggerSpeed, false, false, 1);
	_triggerSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChaseFrame::onTriggerSpeedChanged));

	_bottomBox.pack_start(_transitionSpeedLabel, false, false, 1);
	_bottomBox.pack_start(_transitionSpeed, false, false, 1);
	_transitionSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChaseFrame::onTransitionSpeedChanged));
		
	_bottomBox.pack_start(_synchronizedTriggerCheckButton, false, false, 1);
	_synchronizedTriggerCheckButton.set_group(group);
	_synchronizedTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ChaseFrame::onTriggerTypeChanged));
	_bottomBox.pack_start(_synchronizationsLabel, false, false, 1);
	_bottomBox.pack_start(_synchronizationsCount, false, false, 1);
	_synchronizationsCount.set_value(1.0);
	_synchronizationsCount.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChaseFrame::onSyncCountChanged));

	_bottomBox.pack_start(_beatTriggerCheckButton, false, false, 1);
	_beatTriggerCheckButton.set_group(group);
	_beatTriggerCheckButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ChaseFrame::onTriggerTypeChanged));
	_bottomBox.pack_start(_beatSpeedLabel, false, false, 1);
	_bottomBox.pack_start(_beatSpeed, false, false, 1);
	_beatSpeed.set_value(1.0);
	_beatSpeed.signal_value_changed().
		connect(sigc::mem_fun(*this, &ChaseFrame::onBeatSpeedChanged));
	
	_bottomFrame.add(_bottomBox);

	_bottomFrame.set_sensitive(false);
	_bottomFrame.show_all();
	pack2(_bottomFrame);
}

Chase* ChaseFrame::getSelectedChase()
{
	Gtk::TreeModel::const_iterator selected =
		_chaseListView.get_selection()->get_selected();
	if(selected)
		return (*selected)[_chaseListColumns._chase];
	else
		return nullptr;
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
	if(_delayUpdates.IsFirst())
	{
		Chase* chase = getSelectedChase();
		if(chase)
		{
			_bottomFrame.set_sensitive(true);
			std::unique_lock<std::mutex> lock(_management->Mutex());
			enum Trigger::Type triggerType = chase->Trigger().Type();
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
		}
		else
		{
			_bottomFrame.set_sensitive(false);
		}
	}
}

void ChaseFrame::onDeleteChaseClicked()
{
	Chase* chase = getSelectedChase();
	if(chase)
	{
		_management->RemoveControllable(*chase);
		_parentWindow.EmitUpdateAfterPresetRemoval();
	}
}
