#ifndef FIXTURE_TYPE_FUNCTION_H_
#define FIXTURE_TYPE_FUNCTION_H_

#include "functiontype.h"
#include "fixturefunctionparameters.h"

#include <utility>

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

  MacroParameters& GetMacroParameters() { return parameters_.parameters.macro; }
  const MacroParameters& GetMacroParameters() const {
    return parameters_.parameters.macro;
  }

 private:
  void ConstructParameters() {
    switch (type_) {
      case FunctionType::ColorMacro:
        parameters_.SetMacroParameters(MacroParameters());
        break;
      case FunctionType::Rotation:
        parameters_.SetRotationParameters(RotationParameters());
        break;
      default:
        break;
    }
  }

  void DestructParameters() {
    switch (type_) {
      case FunctionType::ColorMacro:
        parameters_.UnsetMacroParameters();
        break;
      case FunctionType::Rotation:
        parameters_.UnsetRotationParameters();
        break;
      default:
        break;
    }
  }

  void CopyParameters(const FixtureTypeFunction& source) {
    switch (type_) {
      case FunctionType::ColorMacro:
        parameters_.SetMacroParameters(source.parameters_.parameters.macro);
        break;
      case FunctionType::Rotation:
        parameters_.SetRotationParameters(
            source.parameters_.parameters.rotation);
        break;
      default:
        break;
    }
  }

  size_t dmx_offset_;
  FunctionType type_;
  bool is_16_bit_;
  unsigned shape_;
  FixtureFunctionParameters parameters_;
};

#endif
