#ifndef PRESETSFRAME_H
#define PRESETSFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

#include "gui/recursionlock.h"
#include "gui/components/nameframe.h"
#include "gui/components/objectbrowser.h"
#include "gui/mainwindow/objectwindowlist.h"
#include "gui/windows/propertieswindow.h"

#include "theatre/forwards.h"

namespace glight::gui {

class MainWindow;

class ObjectListFrame : public Gtk::VBox {
 public:
  ObjectListFrame(MainWindow &parentWindow);

  theatre::Folder &SelectedFolder() { return _list.SelectedFolder(); }
  void OpenFolder(const theatre::Folder &folder) { _list.OpenFolder(folder); }
  PropertiesWindow &OpenPropertiesWindow(theatre::FolderObject &object);

 private:
  void initPresetsPart();

  void onSelectedObjectChanged();

  ObjectBrowser _list;

  ObjectWindowList<PropertiesWindow> _windowList;

  MainWindow &_parentWindow;
  NameFrame _nameFrame;
  RecursionLock _delayUpdates;
};

}  // namespace glight::gui

#endif
