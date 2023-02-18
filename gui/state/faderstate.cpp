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

FaderState::FaderState(const FaderState &source)
    : source_value_(source.source_value_),
      is_toggle_button_(source.is_toggle_button_),
      new_toggle_button_column_(source.new_toggle_button_column_) {
  if (source_value_ != nullptr)
    source_value_deleted_connection_ = source_value_->SignalDelete().connect(
        [&]() { onPresetValueDeleted(); });
}

FaderState &FaderState::operator=(const FaderState &rhs) {
  source_value_deleted_connection_.disconnect();
  source_value_ = rhs.source_value_;
  if (source_value_ != nullptr)
    source_value_deleted_connection_ = source_value_->SignalDelete().connect(
        [&]() { onPresetValueDeleted(); });
  return *this;
}

void FaderState::SetSourceValue(SourceValue *sourceValue) {
  source_value_deleted_connection_.disconnect();
  source_value_ = sourceValue;
  if (sourceValue != nullptr)
    source_value_deleted_connection_ =
        sourceValue->SignalDelete().connect([&]() { onPresetValueDeleted(); });
}

void FaderState::onPresetValueDeleted() {
  source_value_deleted_connection_.disconnect();
  source_value_ = nullptr;
}

}  // namespace glight::gui
