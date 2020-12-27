#ifndef CURVE_EFFECT_PS_H
#define CURVE_EFFECT_PS_H

#include "propertyset.h"

#include "../effects/curveeffect.h"

#include <string>

class CurveEffectPS final : public PropertySet {
public:
  CurveEffectPS() {
    addProperty(Property(
        "function", "Function",
        std::vector<std::pair<std::string, std::string>>{
            std::pair<std::string, std::string>("linear", "Linear"),
            std::pair<std::string, std::string>("quadratic", "Quadratic"),
            std::pair<std::string, std::string>("exponential", "Exponential"),
            std::pair<std::string, std::string>("logarithmic", "Logarithmic"),
            std::pair<std::string, std::string>("sinusoid", "Sinusoid"),
            std::pair<std::string, std::string>("warmup", "Warm up"),
            std::pair<std::string, std::string>("squareroot", "Square root")}));
  }

protected:
  virtual void setChoice(FolderObject &object, size_t index,
                         const std::string &value) const final override {
    CurveEffect &cfx = static_cast<CurveEffect &>(object);
    switch (index) {
    case 0:
      if (value == "linear")
        cfx.SetFunction(CurveEffect::Linear);
      else if (value == "quadratic")
        cfx.SetFunction(CurveEffect::Quadratic);
      else if (value == "exponential")
        cfx.SetFunction(CurveEffect::Exponential);
      else if (value == "logarithmic")
        cfx.SetFunction(CurveEffect::Logarithmic);
      else if (value == "sinusoid")
        cfx.SetFunction(CurveEffect::Sinusoid);
      else if (value == "warmup")
        cfx.SetFunction(CurveEffect::WarmUp);
      else if (value == "squareroot")
        cfx.SetFunction(CurveEffect::SquareRoot);
      break;
    }
  }

  virtual std::string getChoice(const FolderObject &object,
                                size_t index) const final override {
    const CurveEffect &cfx = static_cast<const CurveEffect &>(object);
    switch (index) {
    case 0:
      switch (cfx.GetFunction()) {
      case CurveEffect::Linear:
        return "linear";
      case CurveEffect::Quadratic:
        return "quadratic";
      case CurveEffect::Exponential:
        return "exponential";
      case CurveEffect::Logarithmic:
        return "logarithmic";
      case CurveEffect::Sinusoid:
        return "sinusoid";
      case CurveEffect::WarmUp:
        return "warmup";
      case CurveEffect::SquareRoot:
        return "squareroot";
      }
      break;
    }
    return 0;
  }
};

#endif
