#ifndef THEATRE_FUNCTION_GENERATOR_EFFECT_H_
#define THEATRE_FUNCTION_GENERATOR_EFFECT_H_

#include <array>

#include "../effect.h"

#include "../timing.h"

namespace glight::theatre {

class FunctionGeneratorEffect final : public Effect {
 public:
  enum class Function {
    Sine,
    Cosine,
    Square,
    Sawtooth,
    Triangle,
    Staircase,
    Strobe
  };

  FunctionGeneratorEffect() : Effect(1) {}

  virtual EffectType GetType() const override {
    return EffectType::FunctionGenerator;
  }

  void SetPeriod(double period) {
    period_ = std::clamp(period, 25.0, 24 * 60.0 * 60.0 * 1000.0);
    next_strobe_time_ = {0.0, 0.0};
  }
  double GetPeriod() const { return period_; }

  void SetFunction(Function function) { function_ = function; }
  Function GetFunction() const { return function_; }

  void SetInvert(bool invert) { invert_ = invert; }
  bool GetInvert() const { return invert_; }

  void SetAmplitude(ControlValue value) { amplitude_ = value; }
  ControlValue GetAmplitude() const { return amplitude_; }

  void SetOffset(ControlValue value) { offset_ = value; }
  ControlValue GetOffset() const { return offset_; }

 protected:
  virtual void MixImplementation(const ControlValue *values,
                                 const Timing &timing, bool primary) override {
    const double phase = std::fmod(timing.TimeInMS(), period_) / period_;
    double output = 0;
    switch (function_) {
      case Function::Sine:
        output = std::sin(phase * 2.0 * M_PI);
        break;
      case Function::Cosine:
        output = std::cos(phase * 2.0 * M_PI);
        break;
      case Function::Square:
        output = phase < 0.5 ? 1.0 : -1.0;
        break;
      case Function::Sawtooth:
        output = phase * 2.0 - 1.0;
        break;
      case Function::Triangle:
        output = phase < 0.5 ? 4.0 * phase - 1.0 : 3.0 - 4.0 * phase;
        break;
      case Function::Staircase:
        output = std::floor(phase * 4.9999) * 0.5 - 1.0;
        break;
      case Function::Strobe:
        if (timing.TimeInMS() >= next_strobe_time_[primary]) {
          next_strobe_time_[primary] = timing.TimeInMS() + period_;
          output = 1.0;
        } else {
          output = -1.0;
        }
        break;
    }
    if (invert_) {
      output = -output;
    }
    const unsigned input = values[0].UInt();
    output =
        std::clamp(output * amplitude_.Ratio() + offset_.Ratio(), 0.0, 1.0) *
        input;
    setAllOutputs(ControlValue(output));
  }

 private:
  Function function_ = Function::Sine;
  bool invert_ = false;
  ControlValue offset_ = ControlValue::Max() / 2;
  ControlValue amplitude_ = ControlValue::Max() / 2;
  double period_ = 750.0;
  std::array<double, 2> next_strobe_time_ = {0.0, 0.0};
};

}  // namespace glight::theatre

#endif
