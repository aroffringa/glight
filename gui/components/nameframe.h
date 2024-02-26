#ifndef GUI_NAMEFRAME_H_
#define GUI_NAMEFRAME_H_

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

#include "theatre/forwards.h"

namespace glight::gui {

class MainWindow;

class NameFrame : public Gtk::HBox {
 public:
  NameFrame();
  ~NameFrame();

  void SetNamedObject(theatre::FolderObject &namedObject) {
    _namedObject = &namedObject;
    update();
  }
  theatre::FolderObject *GetNamedObject() const { return _namedObject; }
  void SetNoNamedObject() {
    _namedObject = nullptr;
    update();
  }
  sigc::signal<void> &SignalNameChange() { return _signalNameChange; }

 private:
  void onButtonClicked();
  void update();

  theatre::FolderObject *_namedObject;

  Gtk::Entry _entry;
  Gtk::Label _label;
  Gtk::Box _buttonBox;
  Gtk::Button _button;
  sigc::signal<void> _signalNameChange;
};

}  // namespace glight::gui

#endif
