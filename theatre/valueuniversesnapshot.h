#ifndef THEATRE_VALUEUNIVERSESNAPSHOT_H_
#define THEATRE_VALUEUNIVERSESNAPSHOT_H_

#include <algorithm>
#include <array>
#include <cmath>

namespace glight::theatre {

constexpr unsigned kChannelsPerUniverse = 512;

class ValueUniverseSnapshot {
 public:
  void SetValues(const unsigned char* values, size_t size) {
    std::copy_n(values, std::min<size_t>(kChannelsPerUniverse, size),
                values_.begin());
  }
  const unsigned char* Data() const { return values_.data(); }
  unsigned char& operator[](size_t index) { return values_[index]; }
  const unsigned char& operator[](size_t index) const { return values_[index]; }

  friend bool operator==(const ValueUniverseSnapshot& left,
                         const ValueUniverseSnapshot& right) {
    return left.values_ == right.values_;
  }

  friend bool operator!=(const ValueUniverseSnapshot& left,
                         const ValueUniverseSnapshot& right) {
    return left.values_ != right.values_;
  }

 private:
  std::array<unsigned char, kChannelsPerUniverse> values_;
};

}  // namespace glight::theatre

#endif  // VALUEUNIVERSESNAPSHOT_H
