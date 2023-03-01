#ifndef THEATRE_SOURCE_VALUE_STORE_H_
#define THEATRE_SOURCE_VALUE_STORE_H_

#include "sourcevalue.h"

#include "../system/smartconnection.h"

#include <cassert>
#include <vector>

namespace glight::theatre {

class Controllable;
class SourceValueStore;

class SourceValueStoreItem {
 public:
  SourceValueStoreItem(SourceValueStore& parent, SourceValue& source_value,
                       ControlValue value)
      : parent_(&parent), value_(value) {
    SetSourceValue(source_value);
  }

  void SetSourceValue(SourceValue& source_value);
  SourceValue& GetSourceValue() const { return *source_value_; }

  ControlValue GetValue() const { return value_; }

 private:
  SourceValueStore* parent_;
  SourceValue* source_value_;
  ControlValue value_ = ControlValue(0u);
  glight::system::SmartConnection connection_;
};

class SourceValueStore {
 public:
  bool Empty() const { return items_.empty(); }

  void Clear() {
    std::vector<SourceValueStoreItem> empty;
    items_.swap(empty);
  }

  void AddItem(SourceValue& source_value, ControlValue value) {
    items_.emplace_back(*this, source_value, value);
  }

  void RemoveItem(SourceValueStoreItem& item) {
    for (std::vector<SourceValueStoreItem>::iterator i = items_.begin();
         i != items_.end(); ++i) {
      if (&*i == &item) {
        items_.erase(i);
        return;
      }
    }
    assert(false);
  }

  const std::vector<SourceValueStoreItem>& GetItems() const { return items_; }

 private:
  std::vector<SourceValueStoreItem> items_;
};

}  // namespace glight::theatre

#endif
