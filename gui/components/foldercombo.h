#ifndef GUI_FOLDER_COMBO_H_
#define GUI_FOLDER_COMBO_H_

#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>

#include "../recursionlock.h"

#include "../../theatre/forwards.h"

namespace glight::gui {

class EventTransmitter;

class FolderCombo : public Gtk::ComboBox {
 public:
  FolderCombo(theatre::Management &management, EventTransmitter &eventHub);

  theatre::Folder &Selection();

  sigc::signal<void()> &SignalSelectionChange() {
    return _signalSelectionChange;
  }

  void Select(const theatre::Folder &object);

 private:
  theatre::Management *_management;
  EventTransmitter &_eventHub;

  Glib::RefPtr<Gtk::ListStore> _listModel;
  struct ListColumns : public Gtk::TreeModelColumnRecord {
    ListColumns() {
      add(_title);
      add(_folder);
    }

    Gtk::TreeModelColumn<Glib::ustring> _title;
    Gtk::TreeModelColumn<theatre::Folder *> _folder;
  } _listColumns;

  void fillList();
  void fillListFolder(const theatre::Folder &folder, size_t depth,
                      const theatre::Folder *selectedObj);
  bool selectObject(const theatre::Folder &object,
                    const Gtk::TreeModel::Children &children);
  void changeManagement(theatre::Management &management) {
    _management = &management;
    fillList();
  }

  sigc::signal<void()> _signalSelectionChange;
  RecursionLock _avoidRecursion;
};

}  // namespace glight::gui

#endif
