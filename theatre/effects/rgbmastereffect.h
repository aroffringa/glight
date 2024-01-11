#ifndef THEATRE_RGB_MASTER_EFFECT_H_
#define THEATRE_RGB_MASTER_EFFECT_H_

#include "../effect.h"

namespace glight::theatre {

/**
 * This effect adds a master channel to R, G and B channels.
 */
class RgbMasterEffect final : public Effect {
 public:
  RgbMasterEffect() : Effect(4) {}

  virtual EffectType GetType() const override { return EffectType::RgbMaster; }

  virtual FunctionType InputType(size_t index) const override {
    constexpr FunctionType type[4] = {FunctionType::Red, FunctionType::Green,
                                      FunctionType::Blue, FunctionType::Master};
    return type[index];
  }

  virtual std::vector<Color> InputColors(size_t index) const override {
    switch (index) {
      default:
      case 0:
        return {Color::RedC()};
      case 1:
        return {Color::GreenC()};
      case 2:
        return {Color::BlueC()};
      case 3:
        return {Color::White()};
    }
  }

  static constexpr size_t kRedInput = 0;
  static constexpr size_t kGreenInput = 1;
  static constexpr size_t kBlueInput = 2;
  static constexpr size_t kMasterInput = 3;

 protected:
  virtual void MixImplementation(const ControlValue *values,
                                 const Timing &timing, bool primary) override {
    for (const std::pair<Controllable *, size_t> &connection : Connections()) {
      const size_t input_index = connection.second;
      const ControlValue master = values[3];
      switch (connection.first->InputType(input_index)) {
        case FunctionType::Red:
          connection.first->MixInput(input_index, values[0] * master);
          break;
        case FunctionType::Green:
          connection.first->MixInput(input_index, values[1] * master);
          break;
        case FunctionType::Blue:
          connection.first->MixInput(input_index, values[2] * master);
          break;
        default:
          break;
      }
    }
  }

 private:
};

}  // namespace glight::theatre

#endif
