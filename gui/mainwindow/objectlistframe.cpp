#include "objectlistframe.h"

#include "gui/mainwindow/actions.h"
#include "gui/mainwindow/mainwindow.h"

#include "theatre/folderobject.h"

namespace glight::gui {

using system::ObservingPtr;

ObjectListFrame::ObjectListFrame(MainWindow &parentWindow)
    : Gtk::Box(Gtk::Orientation::VERTICAL),
      _list(),
      _parentWindow(parentWindow),
      _nameFrame() {
  _list.SignalSelectionChange().connect(
      sigc::mem_fun(*this, &ObjectListFrame::onSelectedObjectChanged));
  _list.SignalObjectActivated().connect(
      [&](ObservingPtr<theatre::FolderObject> object) {
        mainwindow::OpenPropertiesWindow(_windowList, *object, _parentWindow);
      });
  _list.SetDisplayType(ObjectListType::AllExceptFixtures);
  _list.SetShowTypeColumn(true);

  append(_list);
  append(_nameFrame);

  MainMenu &menu = parentWindow.Menu();
  menu.AddEmptyPreset.connect(
      [&]() { mainwindow::NewEmptyPreset(_list, _windowList, _parentWindow); });
  menu.AddCurrentPreset.connect(
      [&]() { mainwindow::NewPresetFromCurrent(_list); });
  menu.AddChase.connect([&]() {
    mainwindow::NewChase(dialog_, _list, _windowList, _parentWindow);
  });
  menu.AddTimeSequence.connect([&]() {
    mainwindow::NewTimeSequence(_list, _windowList, _parentWindow);
  });
  menu.AddEffect.connect([&](theatre::EffectType type) {
    mainwindow::NewEffect(type, _list, _windowList, _parentWindow);
  });
  menu.AddFolder.connect([&]() { mainwindow::NewFolder(_list); });
  menu.DeleteObject.connect([&]() { mainwindow::DeleteObject(_list); });
}

void ObjectListFrame::onSelectedObjectChanged() {
  if (_delayUpdates.IsFirst()) {
    const system::ObservingPtr<theatre::FolderObject> selectedObj =
        _list.SelectedObject();
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
