#include "objectlist.h"

#include "../showwindow.h"

#include "../../theatre/chase.h"
#include "../../theatre/effect.h"
#include "../../theatre/fixturecontrol.h"
#include "../../theatre/folder.h"
#include "../../theatre/management.h"
#include "../../theatre/presetcollection.h"
#include "../../theatre/timesequence.h"

ObjectList::ObjectList(Management &management, ShowWindow &parentWindow) :
	_management(&management),
	_parentWindow(parentWindow),
	_displayType(All),
	_showTypeColumn(false),
	_openFolder(&management.RootFolder()),
	_listView(*this)
{
	_parentWindow.SignalChangeManagement().connect(sigc::mem_fun(*this, &ObjectList::changeManagement));
	
	_parentWindow.SignalUpdateControllables().connect(sigc::mem_fun(*this, &ObjectList::fillList));
	
	_listModel =
    Gtk::ListStore::create(_listColumns);

	_listView.set_model(_listModel);
	_listView.append_column("Object", _listColumns._title);
	_listView.get_selection()->signal_changed().connect([&]()
		{ 
			if(_avoidRecursion.IsFirst())
				_signalSelectionChange.emit(); 
		});
	_listView.signal_row_activated().connect(
		[&](const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn*)
		{
			Gtk::TreeModel::iterator iter = _listModel->get_iter(path);
			if(iter)
				_signalObjectActivated.emit(*(*iter)[_listColumns._object]);
		});
	
	fillList();
	add(_listView);
	set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
}

void ObjectList::SetShowTypeColumn(bool showTypeColumn)
{
	if(showTypeColumn != _showTypeColumn)
	{
		_showTypeColumn = showTypeColumn;
		if(_showTypeColumn)
			_listView.insert_column("T", _listColumns._type, 0);
		else
			_listView.remove_column(*_listView.get_column(0));
	}
}

void ObjectList::fillList()
{
	AvoidRecursion::Token token(_avoidRecursion);
	Glib::RefPtr<Gtk::TreeSelection> selection =
		_listView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	FolderObject* selectedObj = selected ?
		static_cast<FolderObject*>((*selected)[_listColumns._object]) : nullptr;
	_listModel->clear();
	Gtk::TreeViewColumn* objectColumn =
		_showTypeColumn ? _listView.get_column(1) : _listView.get_column(0);
	switch(_displayType)
	{
	case All:
		objectColumn->set_title("objects");
		break;
	case OnlyPresetCollections:
		objectColumn->set_title("preset collections");
		break;
	case OnlyChases:
		objectColumn->set_title("chases");
		break;
	case OnlyEffects:
		objectColumn->set_title("effects");
		break;
	}
	std::unique_lock<std::mutex> lock(_management->Mutex());
	fillListFolder(*_openFolder, selectedObj);
	lock.unlock();
	if(selectedObj && !SelectedObject()) // if the selected object is no longer in the list
		_signalSelectionChange.emit();
}

void ObjectList::fillListFolder(const Folder& folder, const FolderObject* selectedObj)
{
	bool showFolders =
		_displayType==OnlyPresetCollections || _displayType==All;
	bool showPresetCollections =
		_displayType==OnlyPresetCollections || _displayType==All;
	bool showChases =
		_displayType==OnlyChases || _displayType==All;
	bool showEffects =
		_displayType==OnlyEffects || _displayType==All;
		
	for(FolderObject* obj : folder.Children())
	{
		Folder* childFolder = 
			showFolders ? dynamic_cast<Folder*>(obj) : nullptr;
		PresetCollection* presetCollection =
			showPresetCollections ? dynamic_cast<PresetCollection*>(obj) : nullptr;
		Chase* chase =
			showChases ? dynamic_cast<Chase*>(obj) : nullptr;
		TimeSequence* timeSequence =
			showChases ? dynamic_cast<TimeSequence*>(obj) : nullptr;
		Effect* effect =
			showEffects ? dynamic_cast<Effect*>(obj) : nullptr;
		
		if(childFolder || presetCollection || chase || timeSequence || effect)
		{
			Gtk::TreeModel::iterator iter = _listModel->append();
			Gtk::TreeModel::Row childRow = *iter;
			if(chase)
				childRow[_listColumns._type] = "C";
			else if(timeSequence)
				childRow[_listColumns._type] = "T";
			else if(childFolder)
				childRow[_listColumns._type] = "F";
			else if(presetCollection)
				childRow[_listColumns._type] = "P";
			else if(dynamic_cast<FixtureControl*>(obj))
				childRow[_listColumns._type] = "F";
			else if(effect)
				childRow[_listColumns._type] = "E";
			childRow[_listColumns._title] = obj->Name();
			childRow[_listColumns._object] = obj;
			if(obj == selectedObj)
			{
				_listView.get_selection()->select(iter);
			}
		}
	}
}

FolderObject* ObjectList::SelectedObject()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
		_listView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
		return (*selected)[_listColumns._object];
	else
		return nullptr;
}

void ObjectList::SelectObject(const FolderObject& object)
{
	if(!selectObject(object, _listModel->children()))
		throw std::runtime_error("Object to select ('" + object.Name() + "') not found in list");
}

bool ObjectList::selectObject(const FolderObject& object, const Gtk::TreeModel::Children& children)
{
	for(const Gtk::TreeRow& child : children)
	{
		const FolderObject* rowObject = child[_listColumns._object];
		if(rowObject == &object)
		{
			_listView.get_selection()->select(child);
			return true;
		}
	}
	return false;
}

void ObjectList::constructContextMenu()
{
	_contextMenuItems.clear();
	_contextMenu = Gtk::Menu();
	
	FolderObject* obj = SelectedObject();
	if(obj != &_management->RootFolder())
	{
		// Move up & down
		std::unique_ptr<Gtk::MenuItem> miMoveUp(new Gtk::MenuItem("Move _up", true));
		miMoveUp->signal_activate().connect(sigc::mem_fun(this, &ObjectList::onMoveUpSelected));
		_contextMenu.append(*miMoveUp);
		miMoveUp->show();
		_contextMenuItems.emplace_back(std::move(miMoveUp));
		
		std::unique_ptr<Gtk::MenuItem> miMoveDown(new Gtk::MenuItem("Move _down", true));
		miMoveDown->signal_activate().connect(sigc::mem_fun(this, &ObjectList::onMoveDownSelected));
		_contextMenu.append(*miMoveDown);
		miMoveDown->show();
		_contextMenuItems.emplace_back(std::move(miMoveDown));
		
		// Move submenu
		std::unique_ptr<Gtk::Menu> menuMove(new Gtk::Menu());
		menuMove->set_title("Move");
		std::unique_ptr<Gtk::MenuItem> miMove(new Gtk::MenuItem("Move _to", true));
		_contextMenu.append(*miMove);
		miMove->show();
		
		constructFolderMenu(*menuMove, _management->RootFolder());
		miMove->set_submenu(*menuMove);
		
		_contextMenuItems.emplace_back(std::move(miMove));
		_contextMenuItems.emplace_back(std::move(menuMove));
	}
}

void ObjectList::constructFolderMenu(Gtk::Menu& menu, Folder& folder)
{
	std::unique_ptr<Gtk::MenuItem> item(new Gtk::MenuItem(folder.Name()));
	menu.append(*item);
	item->show();
	if(&folder == SelectedObject())
		item->set_sensitive(false);
	else
	{
		Gtk::Menu* subMenu = nullptr;
		for(FolderObject* object : folder.Children())
		{
			Folder* subFolder = dynamic_cast<Folder*>(object);
			if(subFolder)
			{
				if(!subMenu)
				{
					_contextMenuItems.emplace_back(new Gtk::Menu());
					subMenu = static_cast<Gtk::Menu*>(_contextMenuItems.back().get());
					subMenu->set_title(folder.Name());
					item->set_submenu(*subMenu);
					
					std::unique_ptr<Gtk::MenuItem> selfMI(new Gtk::MenuItem("."));
					selfMI->show();
					selfMI->signal_activate().connect([&]() { onMoveSelected(&folder); });
					subMenu->append(*selfMI);
					_contextMenuItems.emplace_back(std::move(selfMI));
				}
				constructFolderMenu(*subMenu, *subFolder);
			}
		}
		if(subMenu == nullptr)
			item->signal_activate().connect([&]() { onMoveSelected(&folder); });
	}
	_contextMenuItems.emplace_back(std::move(item));
}

bool ObjectList::TreeViewWithMenu::on_button_press_event(GdkEventButton* button_event)
{
	bool result = TreeView::on_button_press_event(button_event);
	
	if((button_event->type == GDK_BUTTON_PRESS) && (button_event->button == 3))
  {
		_parent.constructContextMenu();
		_parent._contextMenu.popup_at_pointer((GdkEvent*)button_event);
	}
	
	return result;
}

void ObjectList::onMoveSelected(Folder* destination)
{
	FolderObject* object = SelectedObject();
	Folder::Move(*object, *destination);
	_parentWindow.EmitUpdate();
}

void ObjectList::onMoveUpSelected()
{
	SelectedObject()->Parent().MoveUp(*SelectedObject());
	_parentWindow.EmitUpdate();
}

void ObjectList::onMoveDownSelected()
{
	SelectedObject()->Parent().MoveDown(*SelectedObject());
	_parentWindow.EmitUpdate();
}
