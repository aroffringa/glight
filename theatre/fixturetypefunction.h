#ifndef FIXTURE_TYPE_FUNCTION_H_
#define FIXTURE_TYPE_FUNCTION_H_

#include "functiontype.h"
#include "fixturefunctionparameters.h"

#include <utility>

namespace glight::theatre {

class FixtureTypeFunction {
 public:
  FixtureTypeFunction(FunctionType type, size_t dmx_offset, bool is_16_bit,
                      unsigned shape)
      : type_(type),
        dmx_offset_(dmx_offset),
        is_16_bit_(is_16_bit),
        shape_(shape) {
    ConstructParameters();
  }

  ~FixtureTypeFunction() { DestructParameters(); }

  FixtureTypeFunction(const FixtureTypeFunction& source)
      : type_(source.type_),
        dmx_offset_(source.dmx_offset_),
        is_16_bit_(source.is_16_bit_),
        shape_(source.shape_) {
    CopyParameters(source);
  }

  FixtureTypeFunction& operator=(const FixtureTypeFunction& source) {
    DestructParameters();
    type_ = source.type_;
    dmx_offset_ = source.dmx_offset_;
    is_16_bit_ = source.is_16_bit_;
    shape_ = source.shape_;
    CopyParameters(source);
    return *this;
  }

  size_t DmxOffset() const { return dmx_offset_; }
  void SetDmxOffset(size_t dmx_offset) { dmx_offset_ = dmx_offset; }
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

  FunctionType type_;
  size_t dmx_offset_;
  bool is_16_bit_;
  unsigned shape_;
  FixtureFunctionParameters parameters_;
};

}  // namespace glight::theatre

#endif
