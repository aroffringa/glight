#ifndef GUI_INPUT_SELECT_DIALOG_H_
#define GUI_INPUT_SELECT_DIALOG_H_

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>

#include "theatre/forwards.h"
#include "theatre/input.h"

#include "gui/components/inputselectwidget.h"

namespace glight::gui {

class EventTransmitter;

class InputSelectDialog : public Gtk::Dialog {
 public:
  InputSelectDialog(bool allow_stay_open);

  theatre::Input SelectedInput() const {
    return theatre::Input(*_inputSelector.SelectedObject(),
                          _inputSelector.SelectedInput());
  }

  theatre::SourceValue *SelectedSourceValue() const;

  bool StayOpenRequested() const { return _stayOpenCheckButton.get_active(); }

 private:
  void onSelectionChanged() {
    _selectButton->set_sensitive(_inputSelector.HasInputSelected());
  }
  InputSelectWidget _inputSelector;
  Gtk::CheckButton _stayOpenCheckButton;
  Gtk::Button *_selectButton;
};

}  // namespace glight::gui

#endif
