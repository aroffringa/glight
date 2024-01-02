#ifndef FIXTURE_TYPE_FUNCTION_H_
#define FIXTURE_TYPE_FUNCTION_H_

#include "functiontype.h"
#include "fixturefunctionparameters.h"

#include "system/optionalnumber.h"

#include <utility>

namespace glight::theatre {

class FixtureTypeFunction {
 public:
  /**
   * @param dmx_offset Dmx channel offset of this function from fixture starting
   * channel.
   * @param fine_channel Optional dmx channel offset from fixture starting
   * channel, of the fine channel that corresponds to this function. If set, it
   * implies 16 bits are used for this function.
   */
  FixtureTypeFunction(FunctionType type, size_t dmx_offset,
                      system::OptionalNumber<size_t> fine_channel,
                      unsigned shape)
      : type_(type),
        dmx_offset_(dmx_offset),
        fine_channel_(fine_channel),
        shape_(shape) {
    ConstructParameters();
  }

  ~FixtureTypeFunction() { DestructParameters(); }

  FixtureTypeFunction(const FixtureTypeFunction& source)
      : type_(source.type_),
        dmx_offset_(source.dmx_offset_),
        fine_channel_(source.fine_channel_),
        shape_(source.shape_) {
    CopyParameters(source);
  }

  FixtureTypeFunction& operator=(const FixtureTypeFunction& source) {
    DestructParameters();
    type_ = source.type_;
    dmx_offset_ = source.dmx_offset_;
    fine_channel_ = source.fine_channel_;
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
  /**
   * Optional dmx channel offset from fixture starting channel, of the
   * fine channel that corresponds to this function. If set, it implies 16 bits
   * are used for this function.
   */
  system::OptionalNumber<size_t> FineChannelOffset() const {
    return fine_channel_;
  }
  void SetFineChannelOffset(system::OptionalNumber<size_t> fine_channel) {
    fine_channel_ = fine_channel;
  }

  unsigned Shape() const { return shape_; }

  RotationSpeedParameters& GetRotationParameters() {
    return parameters_.parameters.rotation_speed;
  }
  const RotationSpeedParameters& GetRotationSpeedParameters() const {
    return parameters_.parameters.rotation_speed;
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
      case FunctionType::RotationSpeed:
        parameters_.SetRotationSpeedParameters(RotationSpeedParameters());
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
      case FunctionType::RotationSpeed:
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
      case FunctionType::RotationSpeed:
        parameters_.SetRotationSpeedParameters(
            source.parameters_.parameters.rotation_speed);
        break;
      default:
        break;
    }
  }

  FunctionType type_;
  size_t dmx_offset_;
  system::OptionalNumber<size_t> fine_channel_;
  unsigned shape_;
  FixtureFunctionParameters parameters_;
};

}  // namespace glight::theatre

#endif
