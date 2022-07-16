#include "controlwidget.h"

#include "../../theatre/presetvalue.h"
#include "../../theatre/sourcevalue.h"

namespace glight::gui {

void ControlWidget::setValue(unsigned target) {
  if (GetSourceValue() != nullptr) {
    const unsigned sourceValue = GetSourceValue()->Preset().Value();
    const double fadeSpeed =
        (target > sourceValue) ? _fadeUpSpeed : _fadeDownSpeed;
    GetSourceValue()->Set(target, fadeSpeed);
  }
}

double ControlWidget::MAX_SCALE_VALUE() { return theatre::ControlValue::MaxUInt() + 1; }

}  // namespace glight::gui
