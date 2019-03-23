#include "objecttree.h"

#include "../showwindow.h"

#include "../../libtheatre/chase.h"
#include "../../libtheatre/effect.h"
#include "../../libtheatre/folder.h"
#include "../../libtheatre/management.h"
#include "../../libtheatre/presetcollection.h"
#include "../../libtheatre/sequence.h"

ObjectTree::ObjectTree(Management &management, ShowWindow &parentWindow) :
	_management(&management),
	_parentWindow(parentWindow),
	_displayType(All)
{
	_parentWindow.SignalChangeManagement().connect(sigc::mem_fun(*this, &ObjectTree::changeManagement));
	
	_parentWindow.SignalUpdateControllables().connect(sigc::mem_fun(*this, &ObjectTree::fillList));
	
	_listModel =
    Gtk::TreeStore::create(_listColumns);

	_listView.set_model(_listModel);
	_listView.append_column("Object", _listColumns._title);
	_listView.set_headers_visible(false);
	_listView.get_selection()->signal_changed().connect([&]()
		{ 
			if(_avoidRecursion.IsFirst())
				_signalSelectionChange.emit(); 
		});
	fillList();
	add(_listView);
	set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
}

void ObjectTree::fillList()
{
	AvoidRecursion::Token token(_avoidRecursion);
	Glib::RefPtr<Gtk::TreeSelection> selection =
		_listView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	NamedObject* selectedObj = selected ?
		static_cast<NamedObject*>((*selected)[_listColumns._object]) : nullptr;
	_listModel->clear();

	std::lock_guard<std::mutex> lock(_management->Mutex());
	
	Gtk::TreeModel::iterator iter = _listModel->append();
	Gtk::TreeModel::Row row = *iter;
	switch(_displayType)
	{
		case All:
			row[_listColumns._title] = "all";
			break;
		case OnlyPresetCollections:
			row[_listColumns._title] = "preset collections";
			break;
		case OnlySequences:
			row[_listColumns._title] = "sequences";
			break;
		case PresetsAndSequences:
			row[_listColumns._title] = "presets / sequences";
			break;
		case OnlyChases:
			row[_listColumns._title] = "chases";
			break;
		case OnlyEffects:
			row[_listColumns._title] = "effects";
			break;
	}
	row[_listColumns._object] = &_management->RootFolder();
	if(selectedObj == &_management->RootFolder())
		_listView.get_selection()->select(iter);
	fillListFolder(_management->RootFolder(), row, selectedObj);
	_listView.expand_row(_listModel->get_path(row), false);
	if(selectedObj && !SelectedObject())
		_signalSelectionChange.emit();
}

void ObjectTree::fillListFolder(const Folder& folder, Gtk::TreeModel::Row& row, const NamedObject* selectedObj)
{
	bool showPresetCollections =
		_displayType==OnlyPresetCollections || _displayType==PresetsAndSequences || _displayType==All;
	bool showSequences =
		_displayType==OnlySequences || _displayType==PresetsAndSequences || _displayType==All;
	bool showChases =
		_displayType==OnlyChases || _displayType==All;
	bool showEffects =
		_displayType==OnlyEffects || _displayType==All;
	for(NamedObject* obj : folder.Children())
	{
		Folder* childFolder = dynamic_cast<Folder*>(obj);
		PresetCollection* presetCollection =
			showPresetCollections ? dynamic_cast<PresetCollection*>(obj) : nullptr;
		Sequence* sequence =
			showSequences ? dynamic_cast<Sequence*>(obj) : nullptr;
		Chase* chase =
			showChases ? dynamic_cast<Chase*>(obj) : nullptr;
		Effect* effect =
			showEffects ? dynamic_cast<Effect*>(obj) : nullptr;
		
		if(childFolder || presetCollection || sequence || chase || effect)
		{
			Gtk::TreeModel::iterator iter = _listModel->append(row.children());
			Gtk::TreeModel::Row childRow = *iter;
			childRow[_listColumns._title] = obj->Name();
			childRow[_listColumns._object] = obj;
			if(obj == selectedObj)
			{
				_listView.expand_to_path(_listModel->get_path(iter));
				_listView.get_selection()->select(iter);
			}
			if(childFolder)
				fillListFolder(*childFolder, childRow, selectedObj);
		}
	}
}

NamedObject* ObjectTree::SelectedObject()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
		_listView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
		return (*selected)[_listColumns._object];
	else
		return nullptr;
}

Folder* ObjectTree::SelectedFolder()
{
	NamedObject* selected = SelectedObject();
	if(selected)
	{
		Folder* folder = dynamic_cast<Folder*>(selected);
		// If a folder is selected, return it. If not, return
		// parent folder of selected object.
		if(folder)
			return folder;
		else 
			return &selected->Parent();
	}
	else
		return nullptr;
}

void ObjectTree::SelectObject(const NamedObject& object)
{
	if(!selectObject(object, _listModel->children()))
		throw std::runtime_error("Object to select ('" + object.Name() + "') not found in list");
}

bool ObjectTree::selectObject(const NamedObject& object, const Gtk::TreeModel::Children& children)
{
	for(const Gtk::TreeRow& child : children)
	{
		const NamedObject* rowObject = child[_listColumns._object];
		if(rowObject == &object)
		{
			_listView.expand_to_path(_listModel->get_path(child));
			_listView.get_selection()->select(child);
			return true;
		}
		if(selectObject(object, child.children()))
			return true;
	}
	return false;
}
