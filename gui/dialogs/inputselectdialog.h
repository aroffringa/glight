#ifndef GUI_INPUT_SELECT_DIALOG_H_
#define GUI_INPUT_SELECT_DIALOG_H_

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>

#include "../../theatre/forwards.h"
#include "../../theatre/input.h"

#include "../components/inputselectwidget.h"

namespace glight::gui {

class EventTransmitter;

class InputSelectDialog : public Gtk::Dialog {
 public:
  InputSelectDialog(theatre::Management &management, EventTransmitter &eventHub,
                    bool allow_stay_open)
      : Dialog("Select input", true),
        _management(management),
        _inputSelector(management, eventHub),
        _stayOpenCheckButton("Stay open") {
    set_size_request(600, 400);

    _inputSelector.SignalSelectionChange().connect(
        [&]() { onSelectionChanged(); });
    get_content_area()->pack_start(_inputSelector);
    if (allow_stay_open) {
      get_content_area()->pack_start(_stayOpenCheckButton, false, false);
    }
    add_button("Cancel", Gtk::RESPONSE_CANCEL);
    _selectButton = add_button("Select", Gtk::RESPONSE_OK);
    _selectButton->set_sensitive(false);

    show_all_children();
  }

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
  theatre::Management &_management;
  InputSelectWidget _inputSelector;
  Gtk::CheckButton _stayOpenCheckButton;
  Gtk::Button *_selectButton;
};

}  // namespace glight::gui

#endif
