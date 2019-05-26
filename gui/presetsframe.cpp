#include "presetsframe.h"

#include <gtkmm/stock.h>

#include "showwindow.h"

#include "../libtheatre/chase.h"
#include "../libtheatre/folder.h"
#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"
#include "../libtheatre/presetcollection.h"
#include "../libtheatre/sequence.h"

PresetsFrame::PresetsFrame(Management &management, ShowWindow &parentWindow) :
	_presetsFrame("Preset programming"),
	_presetsList(management, parentWindow),
	_newSequenceFrame("New sequence"),
	_newPresetButton("New preset"),
	_newFolderButton("New folder"),
	_deletePresetButton(Gtk::Stock::DELETE), 
	_addPresetToSequenceButton(Gtk::Stock::ADD),
	_clearSequenceButton("Clear"),
	_createChaseButton("New chase"),
	_management(&management),
	_parentWindow(parentWindow),
	_nameFrame(management, parentWindow)
{
	initPresetsPart();
	initNewSequencePart();

	pack1(_presetsFrame);
	pack2(_newSequenceFrame);
	
	show_all_children();
}

void PresetsFrame::initPresetsPart()
{
	_newPresetButton.set_sensitive(false);
	_newPresetButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onNewPresetButtonClicked));
	_newPresetButton.set_image_from_icon_name("document-new");
	_presetsButtonBox.pack_start(_newPresetButton);

	_newFolderButton.set_sensitive(false);
	_newFolderButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onNewFolderButtonClicked));
	_newFolderButton.set_image_from_icon_name("folder-new");
	_presetsButtonBox.pack_start(_newFolderButton);

	_deletePresetButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onDeletePresetButtonClicked));
	_presetsButtonBox.pack_start(_deletePresetButton);

	_presetsHBox.pack_start(_presetsButtonBox, false, false, 5);
	
	_presetsList.SignalSelectionChange().connect(sigc::mem_fun(this, &PresetsFrame::onSelectedPresetChanged));
	_presetsList.SetDisplayType(ObjectTree::PresetsAndSequences);
	_presetsHBox.pack_start(_presetsList);
	
	_presetsVBox.pack_start(_presetsHBox);
	
	_presetsVBox.pack_start(_nameFrame, false, false, 2);

	_presetsFrame.add(_presetsVBox);
}

void PresetsFrame::initNewSequencePart()
{
	_addPresetToSequenceButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onAddPresetToSequenceButtonClicked));
	_newSequenceButtonBox.pack_start(_addPresetToSequenceButton);
	_addPresetToSequenceButton.set_sensitive(false);

	_clearSequenceButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onClearSequenceButtonClicked));
	_newSequenceButtonBox.pack_start(_clearSequenceButton);

	_createChaseButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onCreateChaseButtonClicked));
	_createChaseButton.set_image_from_icon_name("document-new");
	_newSequenceButtonBox.pack_start(_createChaseButton);

	_newSequenceBox.pack_start(_newSequenceButtonBox, false, false, 5);
	
	_newSequenceListModel =
    Gtk::ListStore::create(_newSequenceListColumns);

	_newSequenceListView.set_model(_newSequenceListModel);
	_newSequenceListView.append_column("Preset", _newSequenceListColumns._title);
	_newSequenceScrolledWindow.add(_newSequenceListView);

	_newSequenceScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_newSequenceBox.pack_start(_newSequenceScrolledWindow);

	_newSequenceFrame.add(_newSequenceBox);
}

void PresetsFrame::onNewPresetButtonClicked()
{
	Folder* parent = _presetsList.SelectedFolder();
	if(parent)
	{
		std::unique_lock<std::mutex> lock(_management->Mutex());
		PresetCollection& presetCollection = _management->AddPresetCollection();
		parent->Add(presetCollection);
		presetCollection.SetFromCurrentSituation(*_management);
		std::stringstream s;
		s << "%" << _management->Controllables().size();
		presetCollection.SetName(s.str());
		_management->AddPreset(presetCollection, 0);
		lock.unlock();

		_parentWindow.EmitUpdate();
		_presetsList.SelectObject(presetCollection);
	}
}

void PresetsFrame::onNewFolderButtonClicked()
{
	Folder* parent = _presetsList.SelectedFolder();
	std::unique_lock<std::mutex> lock(_management->Mutex());
	if(parent)
	{
		Folder& folder = _management->AddFolder(*parent);
		std::stringstream s;
		s << "Folder" << _management->Folders().size();
		folder.SetName(s.str());
		lock.unlock();

		_parentWindow.EmitUpdate();
		_presetsList.SelectObject(folder);
	}
}

void PresetsFrame::onDeletePresetButtonClicked()
{
	FolderObject* selectedObj = _presetsList.SelectedObject();
	if(selectedObj && selectedObj != &_management->RootFolder())
	{
		Folder& parent = selectedObj->Parent();
		{
			std::lock_guard<std::mutex> lock(_management->Mutex());
			_management->RemoveObject(*selectedObj);
		}
		_presetsList.SelectObject(parent);
		_parentWindow.EmitUpdate();
	}
}

void PresetsFrame::onAddPresetToSequenceButtonClicked()
{
	FolderObject* selectedObj = _presetsList.SelectedObject();
	if(selectedObj)
	{
		PresetCollection* preset = dynamic_cast<PresetCollection*>(selectedObj);
		if(preset)
		{
			Gtk::TreeModel::iterator newRow = _newSequenceListModel->append();
			std::lock_guard<std::mutex> lock(_management->Mutex());
			(*newRow)[_newSequenceListColumns._title] = preset->Name();
			(*newRow)[_newSequenceListColumns._preset] = preset;
		}
	}
}

void PresetsFrame::onClearSequenceButtonClicked()
{
	_newSequenceListModel->clear();
}

void PresetsFrame::onCreateChaseButtonClicked()
{
	// Determine folder
	Folder* folder;
	FolderObject* selectedObj = _presetsList.SelectedObject();
	if(selectedObj)
	{
		folder = dynamic_cast<Folder*>(selectedObj);
		if(!folder)
			folder = &selectedObj->Parent();
	}
	else {
		folder = &_management->RootFolder();
	}
	std::unique_lock<std::mutex> lock(_management->Mutex());
	
	Chase& chase = _management->AddChase();
	_management->AddPreset(chase, 0);
	std::stringstream s;
	s << "#" << _management->Controllables().size();
	chase.SetName(s.str());
	folder->Add(chase);

	Sequence& sequence = chase.Sequence();
	Gtk::TreeModel::Children children = _newSequenceListModel->children();
	for(Gtk::TreeModel::Children::const_iterator i=children.begin();
		i != children.end() ; ++i)
	{
		PresetCollection *preset = (*i)[_newSequenceListColumns._preset];
		sequence.Add(preset);
	}

	lock.unlock();

	_parentWindow.EmitUpdate();
	_newSequenceListModel->clear();
	_parentWindow.MakeChaseTabActive(chase);
}

void PresetsFrame::onSelectedPresetChanged()
{
	if(_delayUpdates.IsFirst())
	{
		FolderObject* selectedObj = _presetsList.SelectedObject();
		PresetCollection *preset = nullptr;
		if(selectedObj)
		{
			_nameFrame.SetNamedObject(*selectedObj);
			_deletePresetButton.set_sensitive(selectedObj != &_management->RootFolder());
			_newPresetButton.set_sensitive(true);
			_newFolderButton.set_sensitive(true);
			preset = dynamic_cast<PresetCollection*>(selectedObj);
		}
		else {
			_nameFrame.SetNoNamedObject();
			_deletePresetButton.set_sensitive(false);
			_newPresetButton.set_sensitive(false);
			_newFolderButton.set_sensitive(false);
		}
		if(preset)
			_addPresetToSequenceButton.set_sensitive(true);
		else
			_addPresetToSequenceButton.set_sensitive(false);
	}
}
