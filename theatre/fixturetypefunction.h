#ifndef FIXTURE_TYPE_FUNCTION_H_
#define FIXTURE_TYPE_FUNCTION_H_

#include "functiontype.h"

#include <vector>

struct RotationParameters {
  struct Range {
    unsigned input_min;
    unsigned input_max;
    unsigned speed_min;
    unsigned speed_max;
  };
  std::vector<Range> ranges;
};

struct FixtureTypeParameters {
  void SetRotationParameters(const RotationParameters& rotation_parameters) {
    new (&parameters.rotation_parameters)
        RotationParameters(rotation_parameters);
  }
  void UnsetRotationParameters() {
    parameters.rotation_parameters.~RotationParameters();
  }

  union Parameters {
    Parameters() {}
    ~Parameters() {}
    RotationParameters rotation_parameters;
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
    ConstructParameters();
  }

  FixtureTypeFunction& operator=(const FixtureTypeFunction& source) {
    DestructParameters();
    dmx_offset_ = source.dmx_offset_;
    type_ = source.type_;
    is_16_bit_ = source.is_16_bit_;
    shape_ = source.shape_;
    ConstructParameters();
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

 private:
  void ConstructParameters() {
    if (type_ == FunctionType::Rotation)
      parameters_.SetRotationParameters(RotationParameters());
  }
  void DestructParameters() {
    if (type_ == FunctionType::Rotation) parameters_.UnsetRotationParameters();
  }

  size_t dmx_offset_;
  FunctionType type_;
  bool is_16_bit_;
  unsigned shape_;
  FixtureTypeParameters parameters_;
};

#endif
