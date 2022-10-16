#ifndef THEATRE_VALUESNAPSHOT_H_
#define THEATRE_VALUESNAPSHOT_H_

#include "dmxchannel.h"
#include "valueuniversesnapshot.h"

namespace glight::theatre {

class ValueSnapshot {
 public:
  ValueSnapshot(bool primary, size_t universeCount) : primary_(primary)
  { SetUniverseCount(universeCount); }

  ValueSnapshot(const ValueSnapshot &source) :
    _universeValues(copy(source._universeValues)),
    primary_(source.primary_)
  {}

  ValueSnapshot(ValueSnapshot &&source) = default;

  ValueSnapshot &operator=(const ValueSnapshot &rhs) {
    _universeValues = copy(rhs._universeValues);
    return *this;
  }

  ValueSnapshot &operator=(ValueSnapshot &&rhs) = default;

  ~ValueSnapshot() {}

  void Clear() {
    _universeValues.clear();
  }

  void SetUniverseCount(size_t count) {
    resize(_universeValues, count);
  }

  unsigned char GetValue(const DmxChannel &channel) const {
    return GetUniverseSnapshot(channel.Universe()).GetValue(channel.Channel());
  }

  ValueUniverseSnapshot &GetUniverseSnapshot(size_t index) const {
    return *_universeValues[index];
  }

 private:
  static std::vector<std::unique_ptr<ValueUniverseSnapshot>> copy(const std::vector<std::unique_ptr<ValueUniverseSnapshot>>& source) {
    std::vector<std::unique_ptr<ValueUniverseSnapshot>> result;
    result.reserve(source.size());
    for (const std::unique_ptr<ValueUniverseSnapshot> &snapshot :
         source) {
      result.emplace_back(
          std::make_unique<ValueUniverseSnapshot>(*snapshot));
    }
    return result;
  }
  
  static void resize(std::vector<std::unique_ptr<ValueUniverseSnapshot>>& vec, size_t count) {
    if (count < vec.size()) {
      vec.resize(count);
    }
    else {
      do {
        vec.emplace_back(std::make_unique<ValueUniverseSnapshot>());
      } while (count > vec.size());
    }
  }
   
  std::vector<std::unique_ptr<ValueUniverseSnapshot>> _universeValues;
  bool primary_;
};

}  // namespace glight::theatre

#endif  // VALUESNAPSHOT_H
