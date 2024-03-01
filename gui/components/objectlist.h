#ifndef GUI_OBJECT_TREE_H_
#define GUI_OBJECT_TREE_H_

#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

#include "../recursionlock.h"

#include "../../theatre/forwards.h"

namespace glight::gui {

enum class ObjectListType {
  AllExceptFixtures,
  All,
  OnlyPresetCollections,
  OnlyChases,
  OnlyEffects,
  OnlyVariables
};

class EventTransmitter;

class ObjectList : public Gtk::ScrolledWindow {
 public:
  ObjectList();

  ObjectListType DisplayType() const { return _displayType; }
  void SetDisplayType(ObjectListType displayType) {
    _displayType = displayType;
    fillList();
  }

  theatre::FolderObject *SelectedObject() const;

  std::vector<theatre::FolderObject *> Selection() const;

  sigc::signal<void()> &SignalSelectionChange() {
    return _signalSelectionChange;
  }

  sigc::signal<void(theatre::FolderObject &object)> &SignalObjectActivated() {
    return _signalObjectActivated;
  }

  void SelectObject(const theatre::FolderObject &object);

  void SetFolder(theatre::Folder &folder) {
    if (&folder != _openFolder) {
      _openFolder = &folder;
      bool doChangeSelection =
          (_listView.get_selection()->count_selected_rows() != 0);
      if (doChangeSelection) _listView.get_selection()->unselect_all();
      fillList();
      if (doChangeSelection) _signalSelectionChange.emit();
    }
  }

  void SetShowTypeColumn(bool showTypeColumn);
  bool ShowTypeColumn() const { return _showTypeColumn; }

  void SetAllowMultiSelection(bool allow_multi_selection) {
    _listView.get_selection()->set_mode(allow_multi_selection
                                            ? Gtk::SELECTION_MULTIPLE
                                            : Gtk::SELECTION_SINGLE);
    _listView.set_rubber_banding(allow_multi_selection);
  }

 private:
  enum ObjectListType _displayType = ObjectListType::AllExceptFixtures;
  bool _showFixtureGroups = true;
  bool _showTypeColumn = false;
  theatre::Folder *_openFolder;

  class TreeViewWithMenu : public Gtk::TreeView {
   public:
    TreeViewWithMenu(ObjectList &parent) : _parent(parent) {}

   private:
    ObjectList &_parent;
    bool on_button_press_event(GdkEventButton *button_event) final override;
  } _listView;

  Glib::RefPtr<Gtk::ListStore> _listModel;
  struct ListColumns : public Gtk::TreeModelColumnRecord {
    ListColumns() {
      add(_icon);
      add(_type);
      add(_title);
      add(_object);
    }

    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> _icon;
    Gtk::TreeModelColumn<Glib::ustring> _type;
    Gtk::TreeModelColumn<Glib::ustring> _title;
    Gtk::TreeModelColumn<theatre::FolderObject *> _object;
  } _listColumns;

  void fillList();
  void fillListFolder(const theatre::Folder &folder,
                      const theatre::FolderObject *selectedObj);
  bool selectObject(const theatre::FolderObject &object,
                    const Gtk::TreeModel::Children &children);
  void constructContextMenu();
  void constructFolderMenu(Gtk::Menu &menu, theatre::Folder &folder);
  void onMoveSelected(theatre::Folder *destination);
  void onMoveUpSelected();
  void onMoveDownSelected();

  sigc::signal<void()> _signalSelectionChange;
  sigc::signal<void(theatre::FolderObject &object)> _signalObjectActivated;

  RecursionLock _avoidRecursion;
  Gtk::Menu _contextMenu;
  std::vector<std::unique_ptr<Gtk::Widget>> _contextMenuItems;
};

}  // namespace glight::gui

#endif
