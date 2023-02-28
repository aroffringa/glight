#include "sourcevaluestore.h"

namespace glight::theatre {

void SourceValueStoreItem::SetSourceValue(SourceValue* source_value) {
  source_value_ = source_value;
  connection_ = source_value_->SignalDelete().connect(
      [&]() { parent_->RemoveItem(*this); });
}

}  // namespace glight::theatre
