#ifndef THEATRE_SOURCE_VALUE_STORE_H_
#define THEATRE_SOURCE_VALUE_STORE_H_

#include "sourcevalue.h"

#include "../system/smartconnection.h"

#include <vector>

namespace glight::theatre {

class Controllable;
class SourceValueStore;

class SourceValueStoreItem {
 private:
  SourceValueStoreItem(SourceValueStore& parent) : parent_(&parent) {}

 public:
  void SetSourceValue(SourceValue* source_value);

 private:
  friend class SourceValueStore;
  SourceValueStore* parent_;
  SourceValue* source_value_;
  ControlValue value_ = ControlValue(0u);
  glight::system::SmartConnection connection_;
};

class SourceValueStore {
 public:
  void RemoveItem(SourceValueStoreItem& item) {
    for (std::vector<SourceValueStoreItem>::iterator i = items_.begin();
         i != items_.end(); ++i) {
      if (&*i == &item) {
        items_.erase(i);
        break;
      }
    }
  }

 private:
  std::vector<SourceValueStoreItem> items_;
};

}  // namespace glight::theatre

#endif
