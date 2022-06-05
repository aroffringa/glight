#ifndef FIXTURE_TYPE_FUNCTION_H_
#define FIXTURE_TYPE_FUNCTION_H_

#include "functiontype.h"

#include <algorithm>
#include <utility>
#include <vector>

struct RotationParameters {
  struct Range {
    constexpr Range(unsigned input_min_, unsigned input_max_, int speed_min_,
                    int speed_max_)
        : input_min(input_min_),
          input_max(input_max_),
          speed_min(speed_min_),
          speed_max(speed_max_) {}

    unsigned input_min;
    unsigned input_max;
    int speed_min;
    int speed_max;
  };
  std::vector<Range>& GetRanges() { return ranges_; }
  const std::vector<Range>& GetRanges() const { return ranges_; }
  const Range* GetRange(unsigned input) const {
    auto iter = std::upper_bound(ranges_.begin(), ranges_.end(), input,
                                 [](unsigned lhs, const Range& rhs) -> bool {
                                   return lhs < rhs.input_max;
                                 });
    if (iter == ranges_.end() || iter->input_min > input)
      return nullptr;
    else
      return &*iter;
  }

  Range* GetRange(unsigned input) {
    return const_cast<Range*>(std::as_const(*this).GetRange(input));
  }

  int GetSpeed(unsigned input) const {
    const Range* range = GetRange(input);
    if (!range) {
      return 0;
    } else {
      return int(input - range->input_min) *
                 (range->speed_max - range->speed_min) /
                 int(range->input_max - range->input_min) +
             range->speed_min;
    }
  }

 private:
  // Sorted list that maps input ranges to speed ranges
  std::vector<Range> ranges_;
};

struct FixtureTypeParameters {
  void SetRotationParameters(const RotationParameters& rotation_parameters) {
    new (&parameters.rotation) RotationParameters(rotation_parameters);
  }
  void UnsetRotationParameters() { parameters.rotation.~RotationParameters(); }

  union Parameters {
    Parameters() {}
    ~Parameters() {}
    RotationParameters rotation;
  } parameters;
};

class FixtureTypeFunction {
 public:
  FixtureTypeFunction(size_t dmx_offset, FunctionType type, bool is_16_bit,
                      unsigned shape)
      : dmx_offset_(dmx_offset),
        type_(type),
        is_16_bit_(is_16_bit),
        shape_(shape) {
    ConstructParameters();
  }

  ~FixtureTypeFunction() { DestructParameters(); }

  FixtureTypeFunction(const FixtureTypeFunction& source)
      : dmx_offset_(source.dmx_offset_),
        type_(source.type_),
        is_16_bit_(source.is_16_bit_),
        shape_(source.shape_) {
    CopyParameters(source);
  }

  FixtureTypeFunction& operator=(const FixtureTypeFunction& source) {
    DestructParameters();
    dmx_offset_ = source.dmx_offset_;
    type_ = source.type_;
    is_16_bit_ = source.is_16_bit_;
    shape_ = source.shape_;
    CopyParameters(source);
    return *this;
  }

  size_t DmxOffset() const { return dmx_offset_; }
  FunctionType Type() const { return type_; }
  void SetType(FunctionType type) {
    type_ = type;
    ConstructParameters();
  }
  bool Is16Bit() const { return is_16_bit_; }
  unsigned Shape() const { return shape_; }

  RotationParameters& GetRotationParameters() {
    return parameters_.parameters.rotation;
  }
  const RotationParameters& GetRotationParameters() const {
    return parameters_.parameters.rotation;
  }

 private:
  void ConstructParameters() {
    if (type_ == FunctionType::Rotation)
      parameters_.SetRotationParameters(RotationParameters());
  }
  void DestructParameters() {
    if (type_ == FunctionType::Rotation) parameters_.UnsetRotationParameters();
  }
  void CopyParameters(const FixtureTypeFunction& source) {
    if (type_ == FunctionType::Rotation)
      parameters_.SetRotationParameters(source.parameters_.parameters.rotation);
  }

  size_t dmx_offset_;
  FunctionType type_;
  bool is_16_bit_;
  unsigned shape_;
  FixtureTypeParameters parameters_;
};

#endif
