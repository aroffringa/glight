#include "presetsframe.h"

#include <gtkmm/stock.h>

#include "showwindow.h"
#include "createchasedialog.h"

#include "../libtheatre/chase.h"
#include "../libtheatre/folder.h"
#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"
#include "../libtheatre/presetcollection.h"
#include "../libtheatre/sequence.h"

PresetsFrame::PresetsFrame(Management &management, ShowWindow &parentWindow) :
	_presetsFrame("Preset programming"),
	_presetsList(management, parentWindow),
	_newPresetButton("New preset"),
	_newChaseButton("New chase"),
	_newFolderButton("New folder"),
	_deletePresetButton(Gtk::Stock::DELETE), 
	_management(&management),
	_parentWindow(parentWindow),
	_nameFrame(management, parentWindow)
{
	_parentWindow.SignalChangeManagement().connect(sigc::mem_fun(*this, &PresetsFrame::changeManagement));
	
	initPresetsPart();

	pack1(_presetsFrame);
	//pack2(_newSequenceFrame);
	
	show_all_children();
}

void PresetsFrame::initPresetsPart()
{
	_newPresetButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onNewPresetButtonClicked));
	_newPresetButton.set_image_from_icon_name("document-new");
	_presetsButtonBox.pack_start(_newPresetButton, false, false, 5);

	_newChaseButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onNewChaseButtonClicked));
	_newChaseButton.set_image_from_icon_name("document-new");
	_presetsButtonBox.pack_start(_newChaseButton, false, false, 5);
	
	_newFolderButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onNewFolderButtonClicked));
	_newFolderButton.set_image_from_icon_name("folder-new");
	_presetsButtonBox.pack_start(_newFolderButton, false, false, 5);

	_deletePresetButton.set_sensitive(false);
	_deletePresetButton.signal_clicked().
		connect(sigc::mem_fun(*this, &PresetsFrame::onDeletePresetButtonClicked));
	_presetsButtonBox.pack_start(_deletePresetButton, false, false, 5);

	_presetsHBox.pack_start(_presetsButtonBox, false, false);
	
	_presetsList.SignalSelectionChange().connect(sigc::mem_fun(this, &PresetsFrame::onSelectedPresetChanged));
	_presetsList.SetDisplayType(ObjectList::OnlyPresetCollections);
	_presetsHBox.pack_start(_presetsList);
	
	_presetsVBox.pack_start(_presetsHBox);
	
	_presetsVBox.pack_start(_nameFrame, false, false, 2);

	_presetsFrame.add(_presetsVBox);
}

void PresetsFrame::onNewPresetButtonClicked()
{
	Folder& parent = _presetsList.SelectedFolder();
	std::unique_lock<std::mutex> lock(_management->Mutex());
	PresetCollection& presetCollection = _management->AddPresetCollection();
	parent.Add(presetCollection);
	presetCollection.SetFromCurrentSituation(*_management);
	std::stringstream s;
	s << "%" << _management->Controllables().size();
	presetCollection.SetName(s.str());
	_management->AddPreset(presetCollection, 0);
	lock.unlock();

	_parentWindow.EmitUpdate();
	_presetsList.SelectObject(presetCollection);
}

void PresetsFrame::onNewChaseButtonClicked()
{
	CreateChaseDialog dialog(*_management, _parentWindow);
	dialog.run();
}

void PresetsFrame::onNewFolderButtonClicked()
{
	Folder& parent = _presetsList.SelectedFolder();
	std::unique_lock<std::mutex> lock(_management->Mutex());
	Folder& folder = _management->AddFolder(parent);
	std::stringstream s;
	s << "Folder" << _management->Folders().size();
	folder.SetName(s.str());
	lock.unlock();

	_parentWindow.EmitUpdate();
	_presetsList.OpenFolder(folder);
}

void PresetsFrame::onDeletePresetButtonClicked()
{
	FolderObject* selectedObj = _presetsList.SelectedObject();
	if(selectedObj && selectedObj != &_management->RootFolder())
	{
		{
			std::lock_guard<std::mutex> lock(_management->Mutex());
			_management->RemoveObject(*selectedObj);
		}
		_parentWindow.EmitUpdate();
	}
}

void PresetsFrame::onSelectedPresetChanged()
{
	if(_delayUpdates.IsFirst())
	{
		FolderObject* selectedObj = _presetsList.SelectedObject();
		if(selectedObj)
		{
			_nameFrame.SetNamedObject(*selectedObj);
			_deletePresetButton.set_sensitive(true);
		}
		else {
			_nameFrame.SetNoNamedObject();
			_deletePresetButton.set_sensitive(false);
		}
	}
}
