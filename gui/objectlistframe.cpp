#include "objectlistframe.h"

#include <gtkmm/stock.h>

#include "chasepropertieswindow.h"
#include "createchasedialog.h"
#include "effectpropertieswindow.h"
#include "showwindow.h"

#include "../libtheatre/chase.h"
#include "../libtheatre/folder.h"
#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"
#include "../libtheatre/presetcollection.h"
#include "../libtheatre/sequence.h"

ObjectListFrame::ObjectListFrame(Management &management, ShowWindow &parentWindow) :
	_objectListFrame("Object programming"),
	_list(management, parentWindow),
	_newPresetButton("New preset"),
	_newChaseButton("New chase"),
	_newEffectButton("New effect"), 
	_newFolderButton("New folder"),
	_deletePresetButton(Gtk::Stock::DELETE), 
	_management(&management),
	_parentWindow(parentWindow),
	_nameFrame(management, parentWindow)
{
	_parentWindow.SignalChangeManagement().connect(sigc::mem_fun(*this, &ObjectListFrame::changeManagement));
	
	initPresetsPart();

	pack1(_objectListFrame);
	//pack2(_newSequenceFrame);
	
	show_all_children();
}

void ObjectListFrame::initPresetsPart()
{
	_newPresetButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ObjectListFrame::onNewPresetButtonClicked));
	_newPresetButton.set_image_from_icon_name("document-new");
	_presetsButtonBox.pack_start(_newPresetButton, false, false, 5);

	_newChaseButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ObjectListFrame::onNewChaseButtonClicked));
	_newChaseButton.set_image_from_icon_name("document-new");
	_presetsButtonBox.pack_start(_newChaseButton, false, false, 5);
	
	_newEffectButton.set_events(Gdk::BUTTON_PRESS_MASK);
	_newEffectButton.signal_button_press_event().connect(sigc::mem_fun(*this, &ObjectListFrame::onNewEffectButtonClicked), false);
	_newEffectButton.set_image_from_icon_name("document-new");
	_presetsButtonBox.pack_start(_newEffectButton);

	_newFolderButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ObjectListFrame::onNewFolderButtonClicked));
	_newFolderButton.set_image_from_icon_name("folder-new");
	_presetsButtonBox.pack_start(_newFolderButton, false, false, 5);

	_deletePresetButton.set_sensitive(false);
	_deletePresetButton.signal_clicked().
		connect(sigc::mem_fun(*this, &ObjectListFrame::onDeletePresetButtonClicked));
	_presetsButtonBox.pack_start(_deletePresetButton, false, false, 5);

	_presetsHBox.pack_start(_presetsButtonBox, false, false);
	
	_list.SignalSelectionChange().connect(sigc::mem_fun(this, &ObjectListFrame::onSelectedPresetChanged));
	_list.SignalObjectActivated().connect(sigc::mem_fun(this, &ObjectListFrame::onObjectActivated));
	_list.SetDisplayType(ObjectList::All);
	_list.SetShowTypeColumn(true);
	_presetsHBox.pack_start(_list);
	
	_presetsVBox.pack_start(_presetsHBox);
	
	_presetsVBox.pack_start(_nameFrame, false, false, 2);

	   _objectListFrame.add(_presetsVBox);
}

void ObjectListFrame::onNewPresetButtonClicked()
{
	Folder& parent = _list.SelectedFolder();
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
	_list.SelectObject(presetCollection);
}

void ObjectListFrame::onNewChaseButtonClicked()
{
	CreateChaseDialog dialog(*_management, _parentWindow);
	if(dialog.run() == Gtk::RESPONSE_OK)
	{
		_list.SelectObject(dialog.CreatedChase());
	}
}

bool ObjectListFrame::onNewEffectButtonClicked(GdkEventButton* event)
{
	if(event->button == 1)
	{
		_popupEffectMenuItems.clear();
		_popupEffectMenu.reset(new Gtk::Menu());
	
		std::vector<enum Effect::Type> fxtypes = Effect::GetTypes();
		for(enum Effect::Type t : fxtypes)
		{
			std::unique_ptr<Gtk::MenuItem> mi(new Gtk::MenuItem(Effect::TypeToName(t)));
			mi->signal_activate().connect(sigc::bind<enum Effect::Type>( 
			sigc::mem_fun(*this, &ObjectListFrame::onNewEffectMenuClicked), t));
			_popupEffectMenu->append(*mi);
			_popupEffectMenuItems.emplace_back(std::move(mi));
		}
		
		_popupEffectMenu->show_all_children();
		_popupEffectMenu->popup(event->button, event->time);
		return true;
	}
	return false;
}

void ObjectListFrame::onNewEffectMenuClicked(enum Effect::Type effectType)
{
	std::unique_ptr<Effect> effect(Effect::Make(effectType));
	effect->SetName(Effect::TypeToName(effectType) + std::to_string(_management->Controllables().size()+1));
	Folder& parent = _list.SelectedFolder();
	Effect* added = &_management->AddEffect(std::move(effect), parent);
	for(size_t i=0; i!=added->NInputs(); ++i)
		_management->AddPreset(*added, i);
	_parentWindow.EmitUpdate();
}

void ObjectListFrame::onNewFolderButtonClicked()
{
	Folder& parent = _list.SelectedFolder();
	std::unique_lock<std::mutex> lock(_management->Mutex());
	Folder& folder = _management->AddFolder(parent);
	std::stringstream s;
	s << "Folder" << _management->Folders().size();
	folder.SetName(s.str());
	lock.unlock();

	_parentWindow.EmitUpdate();
	_list.SelectObject(folder);
}

void ObjectListFrame::onDeletePresetButtonClicked()
{
	FolderObject* selectedObj = _list.SelectedObject();
	if(selectedObj && selectedObj != &_management->RootFolder())
	{
		{
			std::lock_guard<std::mutex> lock(_management->Mutex());
			_management->RemoveObject(*selectedObj);
		}
		_parentWindow.EmitUpdate();
	}
}

void ObjectListFrame::onSelectedPresetChanged()
{
	if(_delayUpdates.IsFirst())
	{
		FolderObject* selectedObj = _list.SelectedObject();
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

void ObjectListFrame::onObjectActivated(FolderObject& object)
{
	PropertiesWindow* window = _windowList.GetOpenWindow(object);
	if(window)
	{
		window->present();
	}
	else {
		Chase* chase = dynamic_cast<Chase*>(&object);
		if(chase)
		{
			std::unique_ptr<ChasePropertiesWindow> newWindow(new ChasePropertiesWindow(*chase, *_management, _parentWindow));
			newWindow->present();
			_windowList.Add(std::move(newWindow));
		}
		Effect* effect = dynamic_cast<Effect*>(&object);
		if(effect)
		{
			std::unique_ptr<EffectPropertiesWindow> newWindow(new EffectPropertiesWindow(*effect, *_management, _parentWindow));
			newWindow->present();
			_windowList.Add(std::move(newWindow));
		}
	}
}