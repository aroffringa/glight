#include "nameframe.h"

#include "windows/showwindow.h"

#include "../theatre/folder.h"
#include "../theatre/folderobject.h"
#include "../theatre/management.h"

#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

namespace glight::gui {

NameFrame::NameFrame(theatre::Management &management, ShowWindow &showWindow)
    : _management(&management),
      _showWindow(showWindow),
      _namedObject(nullptr),
      _label("Name:"),
      _button(Gtk::Stock::APPLY) {
  pack_start(_label, false, false, 2);
  _label.show();

  pack_start(_entry, true, true, 2);
  _entry.show();

  _button.signal_clicked().connect(
      sigc::mem_fun(*this, &NameFrame::onButtonClicked));
  _buttonBox.pack_start(_button, false, false, 0);
  _button.show();

  pack_start(_buttonBox, false, false, 2);
  _buttonBox.show();

  update();
}

NameFrame::~NameFrame() {}

void NameFrame::update() {
  if (_namedObject == nullptr) {
    _entry.set_text("");
    set_sensitive(false);
  } else if (_namedObject == &_management->RootFolder()) {
    _entry.set_text("Root");
    set_sensitive(false);
  } else {
    _entry.set_text(_namedObject->Name());
    set_sensitive(true);
  }
}

void NameFrame::onButtonClicked() {
  if (_namedObject != nullptr) {
    const std::string newName = _entry.get_text();

    if (newName != _namedObject->Name()) {
      theatre::FolderObject *folderObject =
          dynamic_cast<theatre::FolderObject *>(_namedObject);
      if (folderObject && !folderObject->IsRoot() &&
          folderObject->Parent().GetChildIfExists(newName)) {
        Gtk::MessageDialog dialog(
            "The folder containing this object already has an object named " +
                newName,
            false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        dialog.run();
      } else {
        std::unique_lock<std::mutex> lock(_management->Mutex());
        _namedObject->SetName(newName);
        lock.unlock();

        _showWindow.EmitUpdate();
        _signalNameChange();
      }
    }
  }
}

}  // namespace glight::gui
