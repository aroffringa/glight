#include "foldercombo.h"

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "theatre/chase.h"
#include "theatre/effect.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"

namespace glight::gui {

using theatre::Folder;
using theatre::FolderObject;

using system::ObservingPtr;

FolderCombo::FolderCombo() : Gtk::ComboBox(false) {
  Instance::Events().SignalUpdateControllables().connect(
      sigc::mem_fun(*this, &FolderCombo::fillList));

  _listModel = Gtk::ListStore::create(_listColumns);

  set_model(_listModel);
  pack_start(_listColumns._title);
  signal_changed().connect([&]() {
    if (_avoidRecursion.IsFirst()) _signalSelectionChange.emit();
  });

  fillList();
}

void FolderCombo::fillList() {
  RecursionLock::Token token(_avoidRecursion);
  Gtk::TreeModel::iterator selected = get_active();
  Folder *selectedObj =
      selected ? static_cast<Folder *>((*selected)[_listColumns._folder])
               : nullptr;
  _listModel->clear();

  theatre::Management &management = Instance::Management();
  std::unique_lock<std::mutex> lock(management.Mutex());

  Gtk::TreeModel::iterator iter = _listModel->append();
  Gtk::TreeModel::Row &row = *iter;
  row[_listColumns._title] = management.RootFolder().Name();
  row[_listColumns._folder] = &management.RootFolder();
  if (selectedObj == &management.RootFolder()) set_active(iter);
  fillListFolder(management.RootFolder(), 1, selectedObj);
  if (!get_active())  // in case it was removed, the selection changes
  {
    lock.unlock();
    Select(management.RootFolder());
    _signalSelectionChange.emit();
  }
}

void FolderCombo::fillListFolder(const Folder &folder, size_t depth,
                                 const Folder *selectedObj) {
  for (const ObservingPtr<FolderObject> &obj : folder.Children()) {
    Folder *childFolder = dynamic_cast<Folder *>(obj.Get());
    if (childFolder) {
      Gtk::TreeModel::iterator iter = _listModel->append();
      Gtk::TreeModel::Row &childRow = *iter;
      childRow[_listColumns._title] = std::string(depth * 3, ' ') + obj->Name();
      childRow[_listColumns._folder] = childFolder;
      if (obj == selectedObj) {
        set_active(iter);
      }
      fillListFolder(*childFolder, depth + 1, selectedObj);
    }
  }
}

Folder &FolderCombo::Selection() const {
  const auto selected = get_active();
  return *(*selected)[_listColumns._folder];
}

void FolderCombo::Select(const Folder &object) {
  Gtk::TreeModel::iterator selected = get_active();
  if (!selected || (*selected)[_listColumns._folder] != &object) {
    if (!selectObject(object, _listModel->children()))
      throw std::runtime_error("Object to select ('" + object.Name() +
                               "') not found in list");
    _signalSelectionChange.emit();
  }
}

bool FolderCombo::selectObject(const Folder &object,
                               const Gtk::TreeModel::ConstChildren &children) {
  for (const Gtk::TreeConstRow &child : children) {
    const Folder *rowObject = child[_listColumns._folder];
    if (rowObject == &object) {
      set_active(child.get_iter());
      return true;
    }
    if (selectObject(object, child.children())) return true;
  }
  return false;
}

}  // namespace glight::gui
