#include "foldercombo.h"

#include "../showwindow.h"

#include "../../libtheatre/chase.h"
#include "../../libtheatre/effect.h"
#include "../../libtheatre/folder.h"
#include "../../libtheatre/management.h"
#include "../../libtheatre/presetcollection.h"

FolderCombo::FolderCombo(Management &management, ShowWindow &parentWindow) :
	Gtk::ComboBox(false),
	_management(&management),
	_parentWindow(parentWindow)
{
	_parentWindow.SignalChangeManagement().connect(sigc::mem_fun(*this, &FolderCombo::changeManagement));
	
	_parentWindow.SignalUpdateControllables().connect(sigc::mem_fun(*this, &FolderCombo::fillList));
	
	_listModel =
    Gtk::ListStore::create(_listColumns);

	set_model(_listModel);
	pack_start(_listColumns._title);
	signal_changed().connect([&]()
		{ 
			if(_avoidRecursion.IsFirst())
				_signalSelectionChange.emit(); 
		});
	
	fillList();
}

void FolderCombo::fillList()
{
	AvoidRecursion::Token token(_avoidRecursion);
	Gtk::TreeModel::iterator selected = get_active();
	Folder* selectedObj = selected ?
		static_cast<Folder*>((*selected)[_listColumns._folder]) : nullptr;
	_listModel->clear();

	std::lock_guard<std::mutex> lock(_management->Mutex());
	
	Gtk::TreeModel::iterator iter = _listModel->append();
	Gtk::TreeModel::Row row = *iter;
	row[_listColumns._title] = _management->RootFolder().Name();
	row[_listColumns._folder] = &_management->RootFolder();
	if(selectedObj == &_management->RootFolder())
		set_active(iter);
	fillListFolder(_management->RootFolder(), 1, selectedObj);
	if(!get_active()) // in case it was removed, the selection changes
	{
		Select(_management->RootFolder());
		_signalSelectionChange.emit();
	}
}

void FolderCombo::fillListFolder(const Folder& folder, size_t depth, const Folder* selectedObj)
{
	for(FolderObject* obj : folder.Children())
	{
		Folder* childFolder = dynamic_cast<Folder*>(obj);
		if(childFolder)
		{
			Gtk::TreeModel::iterator iter = _listModel->append();
			Gtk::TreeModel::Row childRow = *iter;
			childRow[_listColumns._title] = std::string(depth*3, ' ') + obj->Name();
			childRow[_listColumns._folder] = childFolder;
			if(obj == selectedObj)
			{
				set_active(iter);
			}
			fillListFolder(*childFolder, depth+1, selectedObj);
		}
	}
}

Folder& FolderCombo::Selection()
{
	Gtk::TreeModel::iterator selected = get_active();
	return *(*selected)[_listColumns._folder];
}

void FolderCombo::Select(const Folder& object)
{
	if(&Selection() != &object)
	{
		if(!selectObject(object, _listModel->children()))
			throw std::runtime_error("Object to select ('" + object.Name() + "') not found in list");
		_signalSelectionChange.emit();
	}
}

bool FolderCombo::selectObject(const Folder& object, const Gtk::TreeModel::Children& children)
{
	for(const Gtk::TreeRow& child : children)
	{
		const Folder* rowObject = child[_listColumns._folder];
		if(rowObject == &object)
		{
			set_active(child);
			return true;
		}
		if(selectObject(object, child.children()))
			return true;
	}
	return false;
}
