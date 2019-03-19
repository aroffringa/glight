#ifndef OBJECT_TREE_H
#define OBJECT_TREE_H

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treestore.h>
#include <gtkmm/treeview.h>

#include "../avoidrecursion.h"

class ObjectTree : public Gtk::ScrolledWindow
{
public:
	ObjectTree(class Management& management, class ShowWindow& parentWindow);
	
	enum ObjectType {
		All,
		OnlyPresetCollections,
		OnlySequences,
		PresetsAndSequences,
		OnlyChases,
		OnlyEffects
	};
	
	ObjectType DisplayType() const { return _displayType; }
	void SetDisplayType(ObjectType displayType)
	{
		_displayType = displayType;
		fillList();
	}
	
	class NamedObject* SelectedObject();
	
	sigc::signal<void()>& SignalSelectionChange() { return _signalSelectionChange; }
	
	void SelectObject(const NamedObject& object);
	
private:
	class Management* _management;
	class ShowWindow& _parentWindow;
	enum ObjectType _displayType;
	
	Gtk::TreeView _listView;
	Glib::RefPtr<Gtk::TreeStore> _listModel;
	struct ListColumns : public Gtk::TreeModelColumnRecord
	{
		ListColumns()
			{ add(_title); add(_object); }
	
		Gtk::TreeModelColumn<Glib::ustring> _title;
		Gtk::TreeModelColumn<class NamedObject *> _object;
	} _listColumns;
	
	void fillList();
	void fillListFolder(const class Folder& folder, Gtk::TreeModel::Row& row, const class NamedObject* selectedObj);
	bool selectObject(const NamedObject& object, const Gtk::TreeModel::Children& children);
	void changeManagement(class Management &management)
	{
		_management = &management;
		fillList();
	}
	
	sigc::signal<void()> _signalSelectionChange;
	AvoidRecursion _avoidRecursion;
};

#endif
