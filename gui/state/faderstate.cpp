#include "guistate.h"

#include "../../theatre/controllable.h"
#include "../../theatre/management.h"
#include "../../theatre/sourcevalue.h"

namespace glight::gui {

using theatre::SourceValue;

FaderState::FaderState(SourceValue *sourceValue)
    : source_value_(sourceValue),
      is_toggle_button_(false),
      new_toggle_button_column_(false) {
  if (sourceValue != nullptr)
    source_value_deleted_connection_ =
        sourceValue->SignalDelete().connect([&]() { onPresetValueDeleted(); });
}

void FaderState::SetSourceValue(SourceValue *source_value) {
  if (source_value != source_value_) {
    source_value_deleted_connection_.disconnect();
    source_value_ = source_value;
    if (source_value != nullptr)
      source_value_deleted_connection_ = source_value->SignalDelete().connect(
          [&]() { onPresetValueDeleted(); });
    signal_change_();
  }
}

void FaderState::onPresetValueDeleted() {
  source_value_deleted_connection_.disconnect();
  source_value_ = nullptr;
  signal_change_();
}

}  // namespace glight::gui
