#ifndef FUNCTION_GENERATOR_EFFECT_PS_H_
#define FUNCTION_GENERATOR_EFFECT_PS_H_

#include "propertyset.h"

#include "theatre/effects/functiongeneratoreffect.h"

#include <string>

namespace glight::theatre {

class FunctionGeneratorEffectPS final : public PropertySet {
 public:
  FunctionGeneratorEffectPS() {
    addProperty(Property("function", "Function",
                         std::vector<std::pair<std::string, std::string>>{
                             {"sine", "Sine"},
                             {"cosine", "Cosine"},
                             {"square", "Square"},
                             {"sawtooth", "Sawtooth"},
                             {"reverse_sawtooth", "Reverse sawtooth"},
                             {"triange", "Triangle"}}));
    addProperty(Property("invert", "Invert", PropertyType::Boolean));
    addProperty(Property("period", "Period", PropertyType::Duration));
  }

 protected:
  void setDuration(FolderObject &object, size_t index,
                   double value) const override {
    FunctionGeneratorEffect &fgx =
        static_cast<FunctionGeneratorEffect &>(object);
    return fgx.SetPeriod(value);
  }

  double getDuration(const FolderObject &object, size_t index) const override {
    const FunctionGeneratorEffect &fgx =
        static_cast<const FunctionGeneratorEffect &>(object);
    return fgx.GetPeriod();
  }

  void setBool(FolderObject &object, size_t index, bool value) const override {
    FunctionGeneratorEffect &fgx =
        static_cast<FunctionGeneratorEffect &>(object);
    fgx.SetInvert(value);
  }

  bool getBool(const FolderObject &object, size_t index) const override {
    const FunctionGeneratorEffect &fgx =
        static_cast<const FunctionGeneratorEffect &>(object);
    return fgx.GetInvert();
  }

  void setChoice(FolderObject &object, size_t index,
                 const std::string &value) const override {
    FunctionGeneratorEffect &fgx =
        static_cast<FunctionGeneratorEffect &>(object);
    using F = FunctionGeneratorEffect::Function;
    switch (index) {
      case 0:
        if (value == "sine")
          fgx.SetFunction(F::Sine);
        else if (value == "cosine")
          fgx.SetFunction(F::Cosine);
        else if (value == "square")
          fgx.SetFunction(F::Square);
        else if (value == "sawtooth")
          fgx.SetFunction(F::Sawtooth);
        else if (value == "reverse_sawtooth")
          fgx.SetFunction(F::ReverseSawtooth);
        else if (value == "triange")
          fgx.SetFunction(F::Triangle);
        break;
    }
  }

  std::string getChoice(const FolderObject &object,
                        size_t index) const override {
    const FunctionGeneratorEffect &fgx =
        static_cast<const FunctionGeneratorEffect &>(object);
    using F = FunctionGeneratorEffect::Function;
    switch (index) {
      case 0:
        switch (fgx.GetFunction()) {
          case F::Sine:
            return "sine";
          case F::Cosine:
            return "cosine";
          case F::Square:
            return "square";
          case F::Sawtooth:
            return "sawtooth";
          case F::ReverseSawtooth:
            return "reverse_sawtooth";
          case F::Triangle:
            return "triange";
        }
        break;
    }
    return nullptr;
  }
};

}  // namespace glight::theatre

#endif
