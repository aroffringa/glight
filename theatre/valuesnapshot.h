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

  ValueSnapshot(bool primary, size_t universeCount) : is_primary_(primary) {
    SetUniverseCount(universeCount);
  }

  ValueSnapshot(const ValueSnapshot &source)
      : universes_(Copy(source.universes_)), is_primary_(source.is_primary_) {}

  ValueSnapshot(ValueSnapshot &&source) = default;

  ValueSnapshot &operator=(const ValueSnapshot &rhs) {
    if (universes_.size() == rhs.universes_.size()) {
      // This is an optimization to avoid allocations
      for (size_t i = 0; i != universes_.size(); ++i) {
        universes_[i] = rhs.universes_[i];
      }
    } else {
      universes_ = Copy(rhs.universes_);
    }
    is_primary_ = rhs.is_primary_;
    return *this;
  }

  ValueSnapshot &operator=(ValueSnapshot &&rhs) = default;

  ~ValueSnapshot() = default;

  void Clear() { universes_.clear(); }

  size_t UniverseCount() const { return universes_.size(); }
  void SetUniverseCount(size_t count) { Resize(universes_, count); }

  unsigned char GetValue(const DmxChannel &channel) const {
    return channel.Universe() < universes_.size()
               ? universes_[channel.Universe()][channel.Channel()]
               : 0;
  }

  ValueUniverseSnapshot &GetUniverseSnapshot(size_t index) {
    return universes_[index];
  }
  const ValueUniverseSnapshot &GetUniverseSnapshot(size_t index) const {
    return universes_[index];
  }

  friend void swap(ValueSnapshot &left, ValueSnapshot &right) {
    std::swap(left.universes_, right.universes_);
    std::swap(left.is_primary_, right.is_primary_);
  }

  friend bool operator==(const ValueSnapshot &left,
                         const ValueSnapshot &right) {
    if (left.is_primary_ != right.is_primary_) return false;
    if (left.universes_.size() != right.universes_.size()) return false;
    for (size_t i = 0; i != left.universes_.size(); ++i) {
      if (left.universes_[i] != right.universes_[i]) return false;
    }
    return true;
  }

  friend bool operator!=(const ValueSnapshot &left,
                         const ValueSnapshot &right) {
    return !(left == right);
  }

 private:
  static std::vector<ValueUniverseSnapshot> Copy(
      const std::vector<ValueUniverseSnapshot> &source) {
    std::vector<ValueUniverseSnapshot> result;
    result.reserve(source.size());
    for (const ValueUniverseSnapshot &snapshot : source) {
      result.emplace_back(snapshot);
    }
    return result;
  }

  static void Resize(std::vector<ValueUniverseSnapshot> &vec, size_t count) {
    if (count < vec.size()) {
      vec.resize(count);
    } else {
      do {
        vec.emplace_back();
      } while (count > vec.size());
    }
  }

  std::vector<ValueUniverseSnapshot> universes_;
  bool is_primary_ = true;
};

}  // namespace glight::theatre

#endif  // VALUESNAPSHOT_H
