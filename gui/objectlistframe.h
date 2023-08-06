#ifndef PRESETSFRAME_H
#define PRESETSFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

#include "nameframe.h"
#include "recursionlock.h"
#include "windowlist.h"

#include "components/objectbrowser.h"
#include "windows/propertieswindow.h"

#include "../theatre/effect.h"
#include "../theatre/forwards.h"

namespace glight::gui {

class ShowWindow;

class ObjectListFrame : public Gtk::Paned {
 public:
  ObjectListFrame(theatre::Management &management, ShowWindow &parentWindow);

  theatre::Folder &SelectedFolder() { return _list.SelectedFolder(); }
  void OpenFolder(const theatre::Folder &folder) { _list.OpenFolder(folder); }
  PropertiesWindow &OpenPropertiesWindow(theatre::FolderObject &object);

 private:
  void initPresetsPart();

  void onNewPresetButtonClicked();
  void onNewChaseButtonClicked();
  void onNewTimeSequenceButtonClicked();
  bool onNewEffectButtonClicked(GdkEventButton *event);
  void onNewFolderButtonClicked();
  void onDeletePresetButtonClicked();
  void onSelectedObjectChanged();
  void onNewEffectMenuClicked(theatre::EffectType effectType);

  Gtk::Frame _objectListFrame;
  ObjectBrowser _list;

  Gtk::VBox _presetsVBox;
  Gtk::HBox _presetsHBox;

  Gtk::ButtonBox _presetsButtonBox;
  Gtk::Button _newPresetButton, _newChaseButton, _newTimeSequenceButton,
      _newEffectButton, _newFolderButton, _deletePresetButton;

  std::unique_ptr<Gtk::Menu> _popupEffectMenu;
  std::vector<std::unique_ptr<Gtk::MenuItem>> _popupEffectMenuItems;

  WindowList<PropertiesWindow> _windowList;

  theatre::Management *_management;
  ShowWindow &_parentWindow;
  NameFrame _nameFrame;
  RecursionLock _delayUpdates;
};

}  // namespace glight::gui

#endif
