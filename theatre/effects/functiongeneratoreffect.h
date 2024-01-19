#ifndef THEATRE_FUNCTION_GENERATOR_EFFECT_H_
#define THEATRE_FUNCTION_GENERATOR_EFFECT_H_

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
    ReverseSawtooth,
    Triangle
  };

  FunctionGeneratorEffect() : Effect(1) {}

  virtual EffectType GetType() const override {
    return EffectType::FunctionGenerator;
  }

  void SetPeriod(double period) {
    period_ = std::clamp(period, 25.0, 24 * 60.0 * 60.0 * 1000.0);
  }
  double GetPeriod() const { return period_; }

  void SetFunction(Function function) { function_ = function; }
  Function GetFunction() const { return function_; }

  void SetInvert(bool invert) { invert_ = invert; }
  bool GetInvert() const { return invert_; }

 protected:
  virtual void MixImplementation(const ControlValue *values,
                                 const Timing &timing, bool primary) override {
    const double phase = std::fmod(timing.TimeInMS(), period_) / period_;
    const unsigned input = values[0].UInt();
    unsigned output = 0;
    switch (function_) {
      case Function::Sine:
        output = static_cast<unsigned>(
            (std::sin(phase * 2.0 * M_PI) * 0.5 + 0.5) * input);
        break;
      case Function::Cosine:
        output = static_cast<unsigned>(
            (std::cos(phase * 2.0 * M_PI) * 0.5 + 0.5) * input);
        break;
      case Function::Square:
        output = phase < 0.5 ? input : 0.0;
        break;
      case Function::Sawtooth:
        output = static_cast<unsigned>(phase * input);
        break;
      case Function::ReverseSawtooth:
        output = static_cast<unsigned>((1.0 - phase) * input);
        break;
      case Function::Triangle:
        output = phase < 0.5
                     ? static_cast<unsigned>(2.0 * phase * input)
                     : static_cast<unsigned>(2.0 * (1.0 - phase) * input);
        break;
    }
    if (invert_) {
      output = input - output;
    }
    setAllOutputs(ControlValue(output));
  }

 private:
  Function function_ = Function::Sine;
  bool invert_ = false;
  double period_ = 750.0;
};

}  // namespace glight::theatre

#endif
