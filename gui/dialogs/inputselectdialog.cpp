#include "inputselectdialog.h"

#include "gui/instance.h"
#include "theatre/management.h"

namespace glight::gui {

InputSelectDialog::InputSelectDialog(bool allow_stay_open)
    : Dialog("Select input", true),
      _inputSelector(Instance::Get().Management(), Instance::Get().Events()),
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

theatre::SourceValue* InputSelectDialog::SelectedSourceValue() const {
  theatre::Management& management = Instance::Get().Management();
  return management.GetSourceValue(*_inputSelector.SelectedObject(),
                                   _inputSelector.SelectedInput());
}

}  // namespace glight::gui
