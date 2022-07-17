#ifndef THEATRE_VALUEUNIVERSESNAPSHOT_H_
#define THEATRE_VALUEUNIVERSESNAPSHOT_H_

namespace glight::theatre {

class ValueUniverseSnapshot {
 public:
  unsigned char GetValue(size_t channel) const { return _values[channel]; }
  void SetValues(const unsigned char *values, size_t size) {
    if (size > 512) size = 512;
    for (size_t i = 0; i < size; ++i) _values[i] = values[i];
  }

 private:
  unsigned char _values[512];
};

}  // namespace glight::theatre

#endif  // VALUEUNIVERSESNAPSHOT_H
