#ifndef FOLDER_COMBO_H
#define FOLDER_COMBO_H

#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include "../avoidrecursion.h"

class FolderCombo : public Gtk::ComboBox
{
public:
	FolderCombo(class Management& management, class ShowWindow& parentWindow);
	
	class Folder& Selection();
	
	sigc::signal<void()>& SignalSelectionChange() { return _signalSelectionChange; }
	
	void Select(const Folder& object);
	
private:
	class Management* _management;
	class ShowWindow& _parentWindow;
	
	Glib::RefPtr<Gtk::ListStore> _listModel;
	struct ListColumns : public Gtk::TreeModelColumnRecord
	{
		ListColumns()
			{ add(_title); add(_folder); }
	
		Gtk::TreeModelColumn<Glib::ustring> _title;
		Gtk::TreeModelColumn<class Folder *> _folder;
	} _listColumns;
	
	void fillList();
	void fillListFolder(const class Folder& folder, size_t depth, const class Folder* selectedObj);
	bool selectObject(const Folder& object, const Gtk::TreeModel::Children& children);
	void changeManagement(class Management &management)
	{
		_management = &management;
		fillList();
	}
	
	sigc::signal<void()> _signalSelectionChange;
	AvoidRecursion _avoidRecursion;
};

#endif

