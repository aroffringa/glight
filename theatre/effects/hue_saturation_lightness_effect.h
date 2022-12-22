#ifndef THEATRE_HSL_EFFECT_H_
#define THEATRE_HSL_EFFECT_H_

#include "../effect.h"

#include <array>

namespace glight::theatre {

enum class HslColorSpace { LinearHsl, HslUv, CorrectedHsl };

class HueSaturationLightnessEffect final : public Effect {
 public:
  HueSaturationLightnessEffect() : Effect(3) {}

  void test();

  EffectType GetType() const override {
    return EffectType::HueSaturationLightness;
  }

  virtual FunctionType InputType(size_t index) const override {
    switch (index) {
      default:
      case 0:
        return FunctionType::Hue;
      case 1:
        return FunctionType::Saturation;
      case 2:
        return FunctionType::Lightness;
    }
  }

  std::vector<Color> InputColors([[maybe_unused]] size_t index) const override {
    switch (index) {
      default:
      case 0:
        return {Color::RedC()};
      case 1:
        return {};
      case 2:
        return {};
    }
  }

  HslColorSpace ColorSpace() const { return color_space_; }

  void SetColorSpace(HslColorSpace color_space) { color_space_ = color_space; }

 private:
  virtual void mix(const ControlValue *values, const Timing &timing,
                   bool primary) override;

  std::array<ControlValue, 3> Convert(ControlValue l, ControlValue c,
                                      ControlValue h);

  HslColorSpace color_space_ = HslColorSpace::CorrectedHsl;
  const static std::vector<double> table_;
  const static std::vector<double> inverted_table_;
};

}  // namespace glight::theatre

#endif
