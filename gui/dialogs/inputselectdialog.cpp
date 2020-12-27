#include "inputselectdialog.h"

#include "../../theatre/management.h"

PresetValue *InputSelectDialog::SelectedInputPreset() const {
  return _management.GetPresetValue(*_inputSelector.SelectedObject(),
                                    _inputSelector.SelectedInput());
}
