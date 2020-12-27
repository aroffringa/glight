#ifndef INPUT_SELECT_DIALOG_H
#define INPUT_SELECT_DIALOG_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/dialog.h>

#include "../components/inputselectwidget.h"

class InputSelectDialog : public Gtk::Dialog {
public:
  InputSelectDialog(class Management &management,
                    class EventTransmitter &eventHub)
      : Dialog("Select input", true), _management(management),
        _inputSelector(management, eventHub) {
    set_size_request(600, 400);

    _inputSelector.SignalSelectionChange().connect(
        [&]() { onSelectionChanged(); });
    get_content_area()->pack_start(_inputSelector);

    add_button("Cancel", Gtk::RESPONSE_CANCEL);
    _selectButton = add_button("Select", Gtk::RESPONSE_OK);
    _selectButton->set_sensitive(false);

    show_all_children();
  }

  std::pair<Controllable *, size_t> SelectedInput() const {
    return std::make_pair(_inputSelector.SelectedObject(),
                          _inputSelector.SelectedInput());
  }

  class PresetValue *SelectedInputPreset() const;

private:
  void onSelectionChanged() {
    _selectButton->set_sensitive(_inputSelector.HasInputSelected());
  }
  Management &_management;
  InputSelectWidget _inputSelector;
  Gtk::Button *_selectButton;
};

#endif
