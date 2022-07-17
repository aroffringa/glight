#include "inputselectdialog.h"

#include "../../theatre/management.h"

namespace glight::gui {

theatre::SourceValue *InputSelectDialog::SelectedInputPreset() const {
  return _management.GetSourceValue(*_inputSelector.SelectedObject(),
                                    _inputSelector.SelectedInput());
}

}  // namespace glight::gui
