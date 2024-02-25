#ifndef THEATRE_VALUEUNIVERSESNAPSHOT_H_
#define THEATRE_VALUEUNIVERSESNAPSHOT_H_

#include <algorithm>
#include <array>
#include <cmath>

namespace glight::theatre {

class ValueUniverseSnapshot {
 public:
  unsigned char GetValue(size_t channel) const { return values_[channel]; }
  void SetValues(const unsigned char* values, size_t size) {
    std::copy_n(values, std::min<size_t>(512, size), values_.begin());
  }

  friend bool operator==(const ValueUniverseSnapshot& left,
                         const ValueUniverseSnapshot& right) {
    return left.values_ == right.values_;
  }

  friend bool operator!=(const ValueUniverseSnapshot& left,
                         const ValueUniverseSnapshot& right) {
    return left.values_ != right.values_;
  }

 private:
  std::array<unsigned char, 512> values_;
};

}  // namespace glight::theatre

#endif  // VALUEUNIVERSESNAPSHOT_H
