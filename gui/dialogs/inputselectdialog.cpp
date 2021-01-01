#include "inputselectdialog.h"

#include "../../theatre/management.h"

SourceValue *InputSelectDialog::SelectedInputPreset() const {
  return _management.GetSourceValue(*_inputSelector.SelectedObject(),
                                    _inputSelector.SelectedInput());
}
