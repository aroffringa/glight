#ifndef OBJECT_TREE_H
#define OBJECT_TREE_H

#include <gtkmm/menu.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include "../recursionlock.h"

class ObjectList : public Gtk::ScrolledWindow
{
public:
	ObjectList(class Management& management, class EventTransmitter& eventHub);
	
	enum ObjectType {
		AllExceptFixtures,
		All,
		OnlyPresetCollections,
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
	
	sigc::signal<void()>& SignalSelectionChange() { return _signalSelectionChange; }
	
	sigc::signal<void(FolderObject& object)>& SignalObjectActivated()
	{ return _signalObjectActivated; }
	
	void SelectObject(const FolderObject& object);
	
	void SetFolder(class Folder& folder)
	{
		if(&folder != _openFolder)
		{
			_openFolder = &folder;
			bool doChangeSelection = (_listView.get_selection()->count_selected_rows() != 0);
			if(doChangeSelection)
				_listView.get_selection()->unselect_all();
			fillList();
			if(doChangeSelection)
				_signalSelectionChange.emit();
		}
	}
	
	void SetShowTypeColumn(bool showTypeColumn);
	bool ShowTypeColumn() const
	{
		return _showTypeColumn;
	}
	
private:
	class Management* _management;
	class EventTransmitter& _eventHub;
	enum ObjectType _displayType;
	bool _showTypeColumn;
	class Folder* _openFolder;
	
	class TreeViewWithMenu : public Gtk::TreeView
	{
	public:
		TreeViewWithMenu(ObjectList& parent) : _parent(parent) { }
	private:
		ObjectList& _parent;
		bool on_button_press_event(GdkEventButton* button_event) final override;
	} _listView;
	
	Glib::RefPtr<Gtk::ListStore> _listModel;
	struct ListColumns : public Gtk::TreeModelColumnRecord
	{
		ListColumns()
			{ add(_type); add(_title); add(_object); }
	
		Gtk::TreeModelColumn<Glib::ustring> _type;
		Gtk::TreeModelColumn<Glib::ustring> _title;
		Gtk::TreeModelColumn<class FolderObject *> _object;
	} _listColumns;
	
	void fillList();
	void fillListFolder(const class Folder& folder, const class FolderObject* selectedObj);
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
	sigc::signal<void(FolderObject& object)> _signalObjectActivated;
	
	RecursionLock _avoidRecursion;
	Gtk::Menu _contextMenu;
	std::vector<std::unique_ptr<Gtk::Widget>> _contextMenuItems;
};

#endif
