#ifndef THEATRE_VALUESNAPSHOT_H_
#define THEATRE_VALUESNAPSHOT_H_

#include "dmxchannel.h"
#include "valueuniversesnapshot.h"

#include <memory>
#include <vector>

namespace glight::theatre {

class ValueSnapshot {
 public:
  ValueSnapshot() = default;

  ValueSnapshot(bool primary, size_t universeCount) : primary_(primary) {
    SetUniverseCount(universeCount);
  }

  ValueSnapshot(const ValueSnapshot &source)
      : _universeValues(copy(source._universeValues)),
        primary_(source.primary_) {}

  ValueSnapshot(ValueSnapshot &&source) = default;

  ValueSnapshot &operator=(const ValueSnapshot &rhs) {
    if (_universeValues.size() == rhs._universeValues.size()) {
      // This is an optimization to avoid allocations
      for (size_t i = 0; i != _universeValues.size(); ++i) {
        *_universeValues[i] = *rhs._universeValues[i];
      }
    } else {
      _universeValues = copy(rhs._universeValues);
    }
    primary_ = rhs.primary_;
    return *this;
  }

  ValueSnapshot &operator=(ValueSnapshot &&rhs) = default;

  ~ValueSnapshot() = default;

  void Clear() { _universeValues.clear(); }

  size_t UniverseCount() const { return _universeValues.size(); }
  void SetUniverseCount(size_t count) { resize(_universeValues, count); }

  unsigned char GetValue(const DmxChannel &channel) const {
    return GetUniverseSnapshot(channel.Universe())[channel.Channel()];
  }

  ValueUniverseSnapshot &GetUniverseSnapshot(size_t index) const {
    return *_universeValues[index];
  }

  friend void swap(ValueSnapshot &left, ValueSnapshot &right) {
    std::swap(left._universeValues, right._universeValues);
    std::swap(left.primary_, right.primary_);
  }

  friend bool operator==(const ValueSnapshot &left,
                         const ValueSnapshot &right) {
    if (left.primary_ != right.primary_) return false;
    if (left._universeValues.size() != right._universeValues.size())
      return false;
    for (size_t i = 0; i != left._universeValues.size(); ++i) {
      if (*left._universeValues[i] != *right._universeValues[i]) return false;
    }
    return true;
  }

  friend bool operator!=(const ValueSnapshot &left,
                         const ValueSnapshot &right) {
    return !(left == right);
  }

 private:
  static std::vector<std::unique_ptr<ValueUniverseSnapshot>> copy(
      const std::vector<std::unique_ptr<ValueUniverseSnapshot>> &source) {
    std::vector<std::unique_ptr<ValueUniverseSnapshot>> result;
    result.reserve(source.size());
    for (const std::unique_ptr<ValueUniverseSnapshot> &snapshot : source) {
      result.emplace_back(std::make_unique<ValueUniverseSnapshot>(*snapshot));
    }
    return result;
  }

  static void resize(std::vector<std::unique_ptr<ValueUniverseSnapshot>> &vec,
                     size_t count) {
    if (count < vec.size()) {
      vec.resize(count);
    } else {
      do {
        vec.emplace_back(std::make_unique<ValueUniverseSnapshot>());
      } while (count > vec.size());
    }
  }

  std::vector<std::unique_ptr<ValueUniverseSnapshot>> _universeValues;
  bool primary_ = true;
};

}  // namespace glight::theatre

#endif  // VALUESNAPSHOT_H
