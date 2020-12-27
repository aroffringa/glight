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
	ObjectBrowser(class Management& management, class EventTransmitter& eventHub) :
		_folderCombo(management, eventHub),
		_list(management, eventHub)
	{
    _parentFolderButton.set_image_from_icon_name("go-up");
    _parentFolderButton.signal_clicked().connect([&]() { onParentFolderClicked(); });
		_hBox.pack_start(_parentFolderButton, false, false, 5);
    _parentFolderButton.set_sensitive(false);
    _parentFolderButton.show();
    
		_folderCombo.SignalSelectionChange().connect([&]() { onFolderChanged(); });
		_hBox.pack_start(_folderCombo, true, true, 5);
		_folderCombo.show();
    
		pack_start(_hBox, false, false, 5);
    _hBox.show();
		
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
    Folder& folder = _folderCombo.Selection();
    _parentFolderButton.set_sensitive(!folder.IsRoot());
		_list.SetFolder(folder);
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
	void onParentFolderClicked()
  {
    const Folder& folder = _folderCombo.Selection();
    if(!folder.IsRoot())
      _folderCombo.Select(folder.Parent());
  }
	
	Gtk::HBox _hBox;
	Gtk::Button _parentFolderButton;
	FolderCombo _folderCombo;
	ObjectList _list;
	
	sigc::signal<void()> _signalSelectionChange;
	sigc::signal<void()> _signalFolderChange;
	sigc::signal<void(FolderObject& object)> _signalObjectActivated;
};

#endif
