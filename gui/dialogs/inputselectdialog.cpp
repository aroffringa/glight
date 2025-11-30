#include "inputselectdialog.h"

#include "gui/instance.h"
#include "theatre/management.h"

namespace glight::gui {

InputSelectDialog::InputSelectDialog(bool allow_stay_open)
    : Dialog("Select input", true),
      _inputSelector(),
      _stayOpenCheckButton("Stay open") {
  set_size_request(600, 400);

  _inputSelector.SignalSelectionChange().connect(
      [&]() { onSelectionChanged(); });
  get_content_area()->append(_inputSelector);
  if (allow_stay_open) {
    get_content_area()->append(_stayOpenCheckButton);
  }
  add_button("Cancel", Gtk::ResponseType::CANCEL);
  _selectButton = add_button("Select", Gtk::ResponseType::OK);
  _selectButton->set_sensitive(false);
}

theatre::SourceValue* InputSelectDialog::SelectedSourceValue() const {
  theatre::Management& management = Instance::Management();
  return management.GetSourceValue(*_inputSelector.SelectedObject(),
                                   _inputSelector.SelectedInput());
}

}  // namespace glight::gui
