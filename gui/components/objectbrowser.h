#ifndef OBJECT_BROWSER_H
#define OBJECT_BROWSER_H

#include "foldercombo.h"
#include "objectlist.h"

#include "../../theatre/folder.h"
#include "../../theatre/folderobject.h"

#include <gtkmm/box.h>

#include <sigc++/signal.h>

class ObjectBrowser : public Gtk::VBox
{
public:
	ObjectBrowser(class Management& management, class ShowWindow& parentWindow) :
		_folderCombo(management, parentWindow),
		_list(management, parentWindow)
	{
		_folderCombo.SignalSelectionChange().connect([&]() { onFolderChanged(); });
		pack_start(_folderCombo, false, false, 5);
		_folderCombo.show();
		
		_list.SignalSelectionChange().connect([&]() { onSelectionChanged(); });
		_list.SignalObjectActivated().connect([&](FolderObject& object) { onObjectActivated(object); });
		pack_start(_list, true, true);
		_list.show();
	}
	
	ObjectList::ObjectType DisplayType() const { return _list.DisplayType(); }
	void SetDisplayType(ObjectList::ObjectType displayType) { _list.SetDisplayType(displayType); }
	
	void SetShowTypeColumn(bool showTypeColumn) { _list.SetShowTypeColumn(showTypeColumn); }
	bool ShowTypeColumn() const { return _list.ShowTypeColumn(); }
	
	FolderObject* SelectedObject() { return _list.SelectedObject(); }
	
	Folder& SelectedFolder() { return _folderCombo.Selection(); }
	
	sigc::signal<void()>& SignalSelectionChange() { return _signalSelectionChange; }
	
	sigc::signal<void()>& SignalFolderChange() { return _signalFolderChange; }
	
	sigc::signal<void(FolderObject& object)>& SignalObjectActivated() { return _signalObjectActivated; }
	
	void SelectObject(const FolderObject& object)
	{
		_folderCombo.Select(object.Parent());
		_list.SelectObject(object);
	}
	
	void OpenFolder(const Folder& folder)
	{
		_folderCombo.Select(folder);
	}
	
private:
	void onSelectionChanged()
	{
		_signalSelectionChange.emit();
	}
	void onFolderChanged()
	{
		_list.SetFolder(_folderCombo.Selection());
		_signalFolderChange.emit();
	}
	void onObjectActivated(FolderObject& object)
	{
		Folder* folder = dynamic_cast<Folder*>(&object);
		if(folder == nullptr)
			_signalObjectActivated.emit(object);
		else
			_folderCombo.Select(*folder);
	}
	
	FolderCombo _folderCombo;
	ObjectList _list;
	
	sigc::signal<void()> _signalSelectionChange;
	sigc::signal<void()> _signalFolderChange;
	sigc::signal<void(FolderObject& object)> _signalObjectActivated;
};

#endif
