#include "objectlistframe.h"

#include <gtkmm/stock.h>

#include "gui/mainwindow/actions.h"
#include "gui/mainwindow/mainwindow.h"

#include "theatre/folderobject.h"

namespace glight::gui {

ObjectListFrame::ObjectListFrame(MainWindow &parentWindow)
    : _list(), _parentWindow(parentWindow), _nameFrame() {
  _list.SignalSelectionChange().connect(
      sigc::mem_fun(this, &ObjectListFrame::onSelectedObjectChanged));
  _list.SignalObjectActivated().connect(
      [&](glight::theatre::FolderObject &object) {
        mainwindow::OpenPropertiesWindow(_windowList, object, _parentWindow);
      });
  _list.SetDisplayType(ObjectListType::AllExceptFixtures);
  _list.SetShowTypeColumn(true);

  pack_start(_list);
  pack_start(_nameFrame, false, false, 2);

  MainMenu &menu = parentWindow.Menu();
  menu.AddPreset.connect([&]() { mainwindow::NewPreset(_list); });
  menu.AddChase.connect(
      [&]() { mainwindow::NewChase(_list, _windowList, _parentWindow); });
  menu.AddTimeSequence.connect([&]() {
    mainwindow::NewTimeSequence(_list, _windowList, _parentWindow);
  });
  menu.AddEffect.connect([&](theatre::EffectType type) {
    mainwindow::NewEffect(type, _list, _windowList, _parentWindow);
  });
  menu.AddFolder.connect([&]() { mainwindow::NewFolder(_list); });
  menu.DeleteObject.connect([&]() { mainwindow::DeleteObject(_list); });

  show_all_children();
}

void ObjectListFrame::onSelectedObjectChanged() {
  if (_delayUpdates.IsFirst()) {
    theatre::FolderObject *selectedObj = _list.SelectedObject();
    if (selectedObj) {
      _nameFrame.SetNamedObject(*selectedObj);
      _parentWindow.Menu().SetIsObjectSelected(true);
    } else {
      _nameFrame.SetNoNamedObject();
      _parentWindow.Menu().SetIsObjectSelected(false);
    }
  }
}

PropertiesWindow &ObjectListFrame::OpenPropertiesWindow(
    theatre::FolderObject &object) {
  return mainwindow::OpenPropertiesWindow(_windowList, object, _parentWindow);
}

}  // namespace glight::gui
