#include "objectlist.h"

#include <sigc++/signal.h>

#include <gtkmm/gestureclick.h>
#include <gtkmm/icontheme.h>

#include "gui/eventtransmitter.h"
#include "gui/instance.h"

#include "theatre/chase.h"
#include "theatre/effect.h"
#include "theatre/fixturecontrol.h"
#include "theatre/fixturegroup.h"
#include "theatre/folder.h"
#include "theatre/effects/variableeffect.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"
#include "theatre/timesequence.h"

#include "theatre/scenes/scene.h"

namespace glight::gui {

using theatre::Folder;
using theatre::FolderObject;

using system::ObservingPtr;

ObjectList::ObjectList()
    : _openFolder(&Instance::Management().RootFolder()), _listView(*this) {
  Instance::Events().SignalUpdateControllables().connect(
      sigc::mem_fun(*this, &ObjectList::fillList));

  _listModel = Gtk::ListStore::create(_listColumns);

  _listView.set_model(_listModel);
  _listView.append_column("", _listColumns._icon);
  _listView.append_column("object", _listColumns._title);
  _listView.set_enable_search(false);
  _listView.set_expand(true);
  _listView.get_selection()->signal_changed().connect([&]() {
    if (_avoidRecursion.IsFirst()) _signalSelectionChange.emit();
  });
  _listView.signal_row_activated().connect(
      [&](const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *) {
        Gtk::TreeModel::iterator iter = _listModel->get_iter(path);
        if (iter) _signalObjectActivated.emit((*iter)[_listColumns._object]);
      });

  fillList();
  set_child(_listView);
  set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
}

void ObjectList::SetShowTypeColumn(bool showTypeColumn) {
  if (showTypeColumn != _showTypeColumn) {
    _showTypeColumn = showTypeColumn;
    if (_showTypeColumn)
      _listView.insert_column("T", _listColumns._type, 1);
    else
      _listView.remove_column(*_listView.get_column(1));
  }
}

void ObjectList::fillList() {
  RecursionLock::Token token(_avoidRecursion);
  Glib::RefPtr<Gtk::TreeSelection> selection = _listView.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  FolderObject *selectedObj;
  if (selected) {
    const ObservingPtr<FolderObject> &ptr = (*selected)[_listColumns._object];
    selectedObj = ptr.Get();
  } else {
    selectedObj = nullptr;
  }
  _listModel->clear();
  Gtk::TreeViewColumn *objectColumn =
      _showTypeColumn ? _listView.get_column(2) : _listView.get_column(1);
  switch (_displayType) {
    case ObjectListType::AllExceptFixtures:
    case ObjectListType::All:
      objectColumn->set_title("object");
      break;
    case ObjectListType::OnlyPresetCollections:
      objectColumn->set_title("preset collection");
      break;
    case ObjectListType::OnlyChases:
      objectColumn->set_title("chase");
      break;
    case ObjectListType::OnlyEffects:
      objectColumn->set_title("effect");
      break;
    case ObjectListType::OnlyVariables:
      objectColumn->set_title("variables");
      break;
  }
  std::unique_lock<std::mutex> lock(Instance::Management().Mutex());
  fillListFolder(*_openFolder, selectedObj);
  lock.unlock();
  if (selectedObj &&
      !SelectedObject())  // if the selected object is no longer in the list
    _signalSelectionChange.emit();
}

void ObjectList::fillListFolder(const Folder &folder,
                                const FolderObject *selectedObj) {
  bool almostAll = _displayType == ObjectListType::AllExceptFixtures ||
                   _displayType == ObjectListType::All;
  bool showFolders =
      _displayType == ObjectListType::OnlyPresetCollections || almostAll;
  bool showPresetCollections =
      _displayType == ObjectListType::OnlyPresetCollections || almostAll;
  bool showChases = _displayType == ObjectListType::OnlyChases || almostAll;
  bool showEffects = _displayType == ObjectListType::OnlyEffects ||
                     _displayType == ObjectListType::OnlyVariables || almostAll;
  bool showFixtures = _displayType == ObjectListType::All;

  Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::create();
  int icon_height = 16;
  for (ObservingPtr<FolderObject> child : folder.Children()) {
    FolderObject *obj = child.Get();
    Folder *childFolder = showFolders ? dynamic_cast<Folder *>(obj) : nullptr;
    theatre::PresetCollection *presetCollection =
        showPresetCollections ? dynamic_cast<theatre::PresetCollection *>(obj)
                              : nullptr;
    theatre::Chase *chase =
        showChases ? dynamic_cast<theatre::Chase *>(obj) : nullptr;
    theatre::TimeSequence *timeSequence =
        showChases ? dynamic_cast<theatre::TimeSequence *>(obj) : nullptr;
    theatre::Effect *effect = nullptr;
    if (_displayType == ObjectListType::OnlyVariables)
      effect = dynamic_cast<theatre::VariableEffect *>(obj);
    else if (showEffects)
      effect = dynamic_cast<theatre::Effect *>(obj);
    theatre::FixtureControl *fixtureControl =
        showFixtures ? dynamic_cast<theatre::FixtureControl *>(obj) : nullptr;
    theatre::FixtureGroup *fixtureGroup =
        _showFixtureGroups ? dynamic_cast<theatre::FixtureGroup *>(obj)
                           : nullptr;
    theatre::Scene *scene =
        almostAll ? dynamic_cast<theatre::Scene *>(obj) : nullptr;

    if (childFolder || presetCollection || chase || timeSequence || effect ||
        fixtureControl || fixtureGroup || scene) {
      Gtk::TreeModel::iterator iter = _listModel->append();
      Gtk::TreeModel::Row &childRow = *iter;
      std::string icon_name = "x-office-document";
      if (chase) {
        childRow[_listColumns._type] = "C";
        icon_name = "application-x-executable";
      } else if (timeSequence) {
        childRow[_listColumns._type] = "T";
        icon_name = "x-office-calendar";
      } else if (childFolder) {
        childRow[_listColumns._type] = "F";
        icon_name = "folder";
      } else if (presetCollection)
        childRow[_listColumns._type] = "P";
      else if (effect)
        childRow[_listColumns._type] = "E";
      else if (fixtureControl)
        childRow[_listColumns._type] = "L";
      else if (fixtureGroup)
        childRow[_listColumns._type] = "G";
      else if (scene)
        childRow[_listColumns._type] = "S";
      childRow[_listColumns._title] = obj->Name();
      childRow[_listColumns._object] = child;
      if (!icon_name.empty()) {
        childRow[_listColumns._icon] =
            theme->lookup_icon(icon_name, icon_height);
      }
      if (obj == selectedObj) {
        _listView.get_selection()->select(iter);
      }
    }
  }
}

ObservingPtr<FolderObject> ObjectList::SelectedObject() const {
  Glib::RefPtr<const Gtk::TreeSelection> selection = _listView.get_selection();
  const std::vector<Gtk::TreeModel::Path> selected =
      selection->get_selected_rows();
  if (selected.size() == 1) {
    const Gtk::TreeModel::Path path = selected[0];
    return (*_listModel->get_iter(path))[_listColumns._object];
  } else {
    return nullptr;
  }
}

std::vector<ObservingPtr<FolderObject>> ObjectList::Selection() const {
  Glib::RefPtr<const Gtk::TreeSelection> selection = _listView.get_selection();
  const std::vector<Gtk::TreeModel::Path> selected =
      selection->get_selected_rows();
  std::vector<ObservingPtr<FolderObject>> objects;
  objects.reserve(selected.size());
  for (Gtk::TreeModel::Path path : selected) {
    objects.emplace_back((*_listModel->get_iter(path))[_listColumns._object]);
  }
  return objects;
}

void ObjectList::SelectObject(const FolderObject &object) {
  if (!selectObject(object, _listModel->children()))
    throw std::runtime_error("Object to select ('" + object.Name() +
                             "') not found in list");
}

bool ObjectList::selectObject(const FolderObject &object,
                              const Gtk::TreeModel::Children &children) {
  for (const Gtk::TreeConstRow &child : children) {
    const ObservingPtr<FolderObject> &row_object_ptr =
        child[_listColumns._object];
    const FolderObject *row_object = row_object_ptr.Get();
    if (row_object == &object) {
      _listView.get_selection()->select(child.get_iter());
      return true;
    }
  }
  return false;
}

void ObjectList::constructContextMenu() {
  _contextMenu = Gtk::PopoverMenu();
  std::shared_ptr<Gio::Menu> menu = Gio::Menu::create();

  FolderObject *obj = SelectedObject().Get();
  if (obj != &Instance::Management().RootFolder()) {
    std::shared_ptr<Gio::SimpleActionGroup> actions =
        Gio::SimpleActionGroup::create();
    // Move up & down
    menu->append("Move up", "move_object_up");
    actions->add_action("move_object_up",
                        sigc::mem_fun(*this, &ObjectList::onMoveUpSelected));

    menu->append("Move down", "move_object_down");
    actions->add_action("move_object_down",
                        sigc::mem_fun(*this, &ObjectList::onMoveDownSelected));

    // Move-to submenu
    std::shared_ptr<Gio::Menu> move_to_menu = Gio::Menu::create();

    int folder_counter = 0;
    constructFolderMenu(*move_to_menu, *actions,
                        Instance::Management().RootFolder(), folder_counter);
    menu->append_submenu("Move to", move_to_menu);
  }

  _contextMenu.set_menu_model(menu);
}

void ObjectList::constructFolderMenu(Gio::Menu &menu,
                                     Gio::SimpleActionGroup &actions,
                                     Folder &folder, int &counter) {
  if (folder.Children().empty()) {
    const std::string name = "moveto_" + std::to_string(counter);
    ++counter;
    menu.append(folder.Name(), name);
    std::shared_ptr<Gio::SimpleAction> parent =
        actions.add_action(name, [&]() { onMoveSelected(&folder); });

    if (&folder == SelectedObject()) parent->set_enabled(false);
  } else {
    std::shared_ptr<Gio::Menu> sub_menu;
    for (const ObservingPtr<FolderObject> &object : folder.Children()) {
      Folder *subFolder = dynamic_cast<Folder *>(object.Get());
      if (subFolder) {
        if (!sub_menu) {
          sub_menu = Gio::Menu::create();
          const std::string name = "moveto_" + std::to_string(counter);
          ++counter;
          menu.append(".", name);
          actions.add_action(name, [&]() { onMoveSelected(&folder); });
        }
        constructFolderMenu(*sub_menu, actions, *subFolder, counter);
      }
    }
    menu.append_submenu(folder.Name(), sub_menu);
  }
}

ObjectList::TreeViewWithMenu::TreeViewWithMenu(ObjectList &parent)
    : _parent(parent) {
  auto gesture = Gtk::GestureClick::create();
  gesture->set_button(3);
  gesture->signal_pressed().connect(
      [&](int, double, double) { TreeViewWithMenu::OnButtonPress(); });
  add_controller(gesture);
}

void ObjectList::TreeViewWithMenu::OnButtonPress() {
  _parent.constructContextMenu();
  _parent._contextMenu.set_position();
  _parent._contextMenu.popup();
}

void ObjectList::onMoveSelected(Folder *destination) {
  Folder::Move(SelectedObject(), *destination);
  Instance::Events().EmitUpdate();
}

void ObjectList::onMoveUpSelected() {
  SelectedObject()->Parent().MoveUp(*SelectedObject());
  Instance::Events().EmitUpdate();
}

void ObjectList::onMoveDownSelected() {
  SelectedObject()->Parent().MoveDown(*SelectedObject());
  Instance::Events().EmitUpdate();
}

}  // namespace glight::gui
