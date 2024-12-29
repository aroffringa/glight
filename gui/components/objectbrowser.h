#ifndef GUI_OBJECT_BROWSER_H_
#define GUI_OBJECT_BROWSER_H_

#include "foldercombo.h"
#include "objectlist.h"

#include "../../theatre/folder.h"
#include "../../theatre/folderobject.h"
#include "../../theatre/forwards.h"

#include <gtkmm/box.h>

#include <sigc++/signal.h>

namespace glight::gui {

class ObjectBrowser : public Gtk::VBox {
 public:
  ObjectBrowser() : _folderCombo(), _list() {
    _parentFolderButton.set_image_from_icon_name("go-up");
    _parentFolderButton.signal_clicked().connect(
        [&]() { onParentFolderClicked(); });
    _hBox.pack_start(_parentFolderButton, false, false, 5);
    _parentFolderButton.set_sensitive(false);
    _parentFolderButton.show();

    _folderCombo.SignalSelectionChange().connect([&]() { onFolderChanged(); });
    _hBox.pack_start(_folderCombo, true, true, 5);
    _folderCombo.show();

    pack_start(_hBox, false, false, 5);
    _hBox.show();

    _list.SignalSelectionChange().connect([&]() { onSelectionChanged(); });
    _list.SignalObjectActivated().connect(
        [&](ObservingPtr<theatre::FolderObject> object) {
          onObjectActivated(object);
        });
    pack_start(_list, true, true);
    _list.show();
  }

  ObjectListType DisplayType() const { return _list.DisplayType(); }
  void SetDisplayType(ObjectListType displayType) {
    _list.SetDisplayType(displayType);
  }

  void SetShowTypeColumn(bool showTypeColumn) {
    _list.SetShowTypeColumn(showTypeColumn);
  }
  bool ShowTypeColumn() const { return _list.ShowTypeColumn(); }

  system::ObservingPtr<theatre::FolderObject> SelectedObject() const {
    return _list.SelectedObject();
  }

  std::vector<system::ObservingPtr<theatre::FolderObject>> Selection() const {
    return _list.Selection();
  }

  theatre::Folder &SelectedFolder() { return _folderCombo.Selection(); }

  sigc::signal<void()> &SignalSelectionChange() {
    return _signalSelectionChange;
  }

  sigc::signal<void()> &SignalFolderChange() { return _signalFolderChange; }

  sigc::signal<void(system::ObservingPtr<theatre::FolderObject> object)>
      &SignalObjectActivated() {
    return _signalObjectActivated;
  }

  void SelectObject(const theatre::FolderObject &object) {
    _folderCombo.Select(object.Parent());
    _list.SelectObject(object);
  }

  void OpenFolder(const theatre::Folder &folder) {
    _folderCombo.Select(folder);
  }

  void SetAllowMultiSelection(bool allow_multi_selection) {
    _list.SetAllowMultiSelection(allow_multi_selection);
  }

 private:
  void onSelectionChanged() { _signalSelectionChange.emit(); }
  void onFolderChanged() {
    theatre::Folder &folder = _folderCombo.Selection();
    _parentFolderButton.set_sensitive(!folder.IsRoot());
    _list.SetFolder(folder);
    _signalFolderChange.emit();
  }
  void onObjectActivated(system::ObservingPtr<theatre::FolderObject> object) {
    theatre::Folder *folder = dynamic_cast<theatre::Folder *>(object.Get());
    if (folder == nullptr)
      _signalObjectActivated.emit(object);
    else
      _folderCombo.Select(*folder);
  }
  void onParentFolderClicked() {
    const theatre::Folder &folder = _folderCombo.Selection();
    if (!folder.IsRoot()) _folderCombo.Select(folder.Parent());
  }

  Gtk::HBox _hBox;
  Gtk::Button _parentFolderButton;
  FolderCombo _folderCombo;
  ObjectList _list;

  sigc::signal<void()> _signalSelectionChange;
  sigc::signal<void()> _signalFolderChange;
  sigc::signal<void(system::ObservingPtr<theatre::FolderObject> object)>
      _signalObjectActivated;
};

}  // namespace glight::gui

#endif
