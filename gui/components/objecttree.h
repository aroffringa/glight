#ifndef OBJECT_TREE_H
#define OBJECT_TREE_H

#include <gtkmm/menu.h>
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
	
	class FolderObject* SelectedObject();
	
	class Folder* SelectedFolder();
	
	sigc::signal<void()>& SignalSelectionChange() { return _signalSelectionChange; }
	
	void SelectObject(const FolderObject& object);
	
private:
	class Management* _management;
	class ShowWindow& _parentWindow;
	enum ObjectType _displayType;
	
	class TreeViewWithMenu : public Gtk::TreeView
	{
	public:
		TreeViewWithMenu(ObjectTree& parent) : _parent(parent) { }
	private:
		ObjectTree& _parent;
		bool on_button_press_event(GdkEventButton* button_event) final override;
	} _listView;
	
	Glib::RefPtr<Gtk::TreeStore> _listModel;
	struct ListColumns : public Gtk::TreeModelColumnRecord
	{
		ListColumns()
			{ add(_title); add(_object); }
	
		Gtk::TreeModelColumn<Glib::ustring> _title;
		Gtk::TreeModelColumn<class FolderObject *> _object;
	} _listColumns;
	
	void fillList();
	void fillListFolder(const class Folder& folder, Gtk::TreeModel::Row& row, const class FolderObject* selectedObj);
	bool selectObject(const FolderObject& object, const Gtk::TreeModel::Children& children);
	void changeManagement(class Management &management)
	{
		_management = &management;
		fillList();
	}
	void constructContextMenu();
	void constructFolderMenu(Gtk::Menu& menu, Folder& folder);
	void onMoveSelected(Folder* destination);
	void onMoveUpSelected();
	void onMoveDownSelected();
	
	sigc::signal<void()> _signalSelectionChange;
	AvoidRecursion _avoidRecursion;
	Gtk::Menu _contextMenu;
	std::vector<std::unique_ptr<Gtk::Widget>> _contextMenuItems;
};

#endif
