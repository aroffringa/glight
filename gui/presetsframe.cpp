#include "presetsframe.h"

#include <gtkmm/stock.h>

#include "programwindow.h"

#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"
#include "../libtheatre/presetcollection.h"
#include "../libtheatre/sequence.h"

PresetsFrame::PresetsFrame(Management &management, ProgramWindow &parentWindow) :
	_presetsFrame("Preset programming"),
	_newSequenceFrame("New sequence"),
	_newPresetButton(Gtk::Stock::NEW), 
	_deletePresetButton(Gtk::Stock::DELETE), 
	_addPresetToSequenceButton(Gtk::Stock::ADD),
	_clearSequenceButton("Clear"),
	_createSequenceButton("Create"),
	_management(&management),
	_parentWindow(parentWindow),
	_nameFrame(management)
{
	initPresetsPart();
	initNewSequencePart();

	pack1(_presetsFrame);
	_presetsFrame.show();

	pack2(_newSequenceFrame);
	_newSequenceFrame.show();
}

PresetsFrame::~PresetsFrame()
{
}

void PresetsFrame::initPresetsPart()
{
	_newPresetButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onNewPresetButtonClicked));
	_presetsButtonBox.pack_start(_newPresetButton);
	_newPresetButton.show();

	_deletePresetButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onDeletePresetButtonClicked));
	_presetsButtonBox.pack_start(_deletePresetButton);
	_deletePresetButton.show();

	_presetsHBox.pack_start(_presetsButtonBox, false, false, 5);
	_presetsButtonBox.show();

	_presetListModel =
    Gtk::ListStore::create(_presetListColumns);

	_presetListView.set_model(_presetListModel);
	_presetListView.append_column("Preset", _presetListColumns._title);
	_presetListView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &PresetsFrame::onSelectedPresetChanged));
	FillPresetsList();
	_presetsScrolledWindow.add(_presetListView);
	_presetListView.show();

	_presetsScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_presetsHBox.pack_start(_presetsScrolledWindow);
	_presetsScrolledWindow.show();

	_presetsVBox.pack_start(_presetsHBox);
	_presetsHBox.show();

	_nameFrame.SignalNameChange().connect(sigc::mem_fun(*this, &PresetsFrame::onNameChange));
	_presetsVBox.pack_start(_nameFrame, false, false, 2);
	_nameFrame.show();

	_presetsFrame.add(_presetsVBox);
	_presetsVBox.show();
}

void PresetsFrame::initNewSequencePart()
{
	_addPresetToSequenceButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onAddPresetToSequenceButtonClicked));
	_newSequenceButtonBox.pack_start(_addPresetToSequenceButton);
	_addPresetToSequenceButton.set_sensitive(false);
	_addPresetToSequenceButton.show();

	_clearSequenceButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onClearSequenceButtonClicked));
	_newSequenceButtonBox.pack_start(_clearSequenceButton);
	_clearSequenceButton.show();

	_createSequenceButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onCreateSequenceButtonClicked));
	_newSequenceButtonBox.pack_start(_createSequenceButton);
	_createSequenceButton.show();

	_newSequenceBox.pack_start(_newSequenceButtonBox, false, false, 5);
	_newSequenceButtonBox.show();
	
	_newSequenceListModel =
    Gtk::ListStore::create(_newSequenceListColumns);

	_newSequenceListView.set_model(_newSequenceListModel);
	_newSequenceListView.append_column("Preset", _newSequenceListColumns._title);
	_newSequenceScrolledWindow.add(_newSequenceListView);
	_newSequenceListView.show();

	_newSequenceScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_newSequenceBox.pack_start(_newSequenceScrolledWindow);
	_newSequenceScrolledWindow.show();

	_newSequenceFrame.add(_newSequenceBox);
	_newSequenceBox.show();
}

void PresetsFrame::FillPresetsList()
{
	AvoidRecursion::Token token(_delayUpdates);
	_presetListModel->clear();

	std::lock_guard<std::mutex> lock(_management->Mutex());
	const std::vector<std::unique_ptr<Controllable>>&
		controllables = _management->Controllables();
	for(const std::unique_ptr<Controllable>& contr : controllables)
	{
		PresetCollection *presetCollection =
			dynamic_cast<PresetCollection*>(contr.get());
		if(presetCollection != nullptr)
		{
			Gtk::TreeModel::iterator iter = _presetListModel->append();
			Gtk::TreeModel::Row row = *iter;
			row[_presetListColumns._title] = presetCollection->Name();
			row[_presetListColumns._preset] = presetCollection;
		}
	}
}

void PresetsFrame::onNewPresetButtonClicked()
{
	std::unique_lock<std::mutex> lock(_management->Mutex());
	PresetCollection &presetCollection = _management->AddPresetCollection();
	presetCollection.SetFromCurrentSituation(*_management);
	std::stringstream s;
	s << "%" << _presetListModel->children().size();
	presetCollection.SetName(s.str());
	_management->AddPreset(presetCollection);
	lock.unlock();

	FillPresetsList();
	_parentWindow.ForwardUpdateAfterAddPreset();
}

void PresetsFrame::onDeletePresetButtonClicked()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
    _presetListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
	{
		PresetCollection *preset = (*selected)[_presetListColumns._preset];
		_presetListModel->erase(selected);
		std::unique_lock<std::mutex> lock(_management->Mutex());
		_management->RemoveControllable(*preset);
		lock.unlock();
	
		_parentWindow.ForwardUpdateAfterPresetRemoval();
	}
}

void PresetsFrame::onAddPresetToSequenceButtonClicked()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
    _presetListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
	{
		PresetCollection *preset = (*selected)[_presetListColumns._preset];
		Gtk::TreeModel::iterator newRow = _newSequenceListModel->append();
		std::lock_guard<std::mutex> lock(_management->Mutex());
		(*newRow)[_newSequenceListColumns._title] = preset->Name();
		(*newRow)[_newSequenceListColumns._preset] = preset;
	}
}

void PresetsFrame::onClearSequenceButtonClicked()
{
	_newSequenceListModel->clear();
}

void PresetsFrame::onCreateSequenceButtonClicked()
{
	std::unique_lock<std::mutex> lock(_management->Mutex());
	Sequence &sequence = _management->AddSequence();

	Gtk::TreeModel::Children children = _newSequenceListModel->children();
	for(Gtk::TreeModel::Children::const_iterator i=children.begin();
		i != children.end() ; ++i)
	{
		PresetCollection *preset = (*i)[_newSequenceListColumns._preset];
		sequence.AddPreset(preset);
	}

	std::stringstream s;
	s << "#" << _management->Sequences().size();
	sequence.SetName(s.str());
	lock.unlock();

	_parentWindow.UpdateSequenceList();
	_parentWindow.MakeSequenceTabActive();
	_newSequenceListModel->clear();
}

void PresetsFrame::onSelectedPresetChanged()
{
	if(_delayUpdates.IsFirst())
	{
		Glib::RefPtr<Gtk::TreeSelection> selection =
			_presetListView.get_selection();
		Gtk::TreeModel::iterator selected = selection->get_selected();
		if(selected)
		{
			PresetCollection *preset = (*selected)[_presetListColumns._preset];
			_nameFrame.SetNamedObject(*preset);

			_addPresetToSequenceButton.set_sensitive(true);
		}
		else
		{
			_nameFrame.SetNoNamedObject();

			_addPresetToSequenceButton.set_sensitive(false);
		}
	}
}
