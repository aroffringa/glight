#ifndef THEATRE_TRANSITION_H_
#define THEATRE_TRANSITION_H_

#include "presetcollection.h"
#include "timing.h"

namespace glight::theatre {

enum class TransitionType {
  None,
  Fade,
  FadeThroughBlack,
  Erratic,
  Black,
  FadeFromBlack,
  FadeToBlack
};

inline std::string ToString(TransitionType mix_style) {
  switch (mix_style) {
    default:
    case TransitionType::None:
      return "none";
    case TransitionType::Fade:
      return "fade";
    case TransitionType::FadeThroughBlack:
      return "fade_through_black";
    case TransitionType::Erratic:
      return "erratic";
    case TransitionType::Black:
      return "black";
    case TransitionType::FadeFromBlack:
      return "fade_from_black";
    case TransitionType::FadeToBlack:
      return "fade_to_black";
  }
}

inline TransitionType GetTransitionType(const std::string &str) {
  if (str == "fade")
    return TransitionType::Fade;
  else if (str == "fade_through_black")
    return TransitionType::FadeThroughBlack;
  else if (str == "erratic")
    return TransitionType::Erratic;
  else if (str == "black")
    return TransitionType::Black;
  else if (str == "fade_from_black")
    return TransitionType::FadeFromBlack;
  else if (str == "fade_to_black")
    return TransitionType::FadeToBlack;
  else  // "none"
    return TransitionType::None;
}

/**
 * @author Andre Offringa
 */
class Transition {
 public:
  Transition() : _lengthInMs(250.0), _type(TransitionType::Fade) {}
  ~Transition() {}

  TransitionType Type() const { return _type; }
  void SetType(TransitionType type) { _type = type; }

  double LengthInMs() const { return _lengthInMs; }
  void SetLengthInMs(double length) { _lengthInMs = length; }

  /**
   * @param transitionTime value between 0 and _lengthInMS.
   */
  void Mix(Controllable &first, size_t firstInput, Controllable &second,
           size_t secondInput, double transitionTime, const ControlValue &value,
           const Timing &timing) const {
    switch (_type) {
      case TransitionType::None:
        if (transitionTime * 2.0 <= _lengthInMs)
          first.MixInput(firstInput, value);
        else
          second.MixInput(secondInput, value);
        break;
      case TransitionType::Fade: {
        unsigned secondRatioValue =
            (unsigned)((transitionTime / _lengthInMs) * 256.0);
        unsigned firstRatioValue = 255 - secondRatioValue;
        first.MixInput(firstInput, (value.UInt() * firstRatioValue) >> 8);
        second.MixInput(secondInput, (value.UInt() * secondRatioValue) >> 8);
      } break;
      case TransitionType::FadeThroughBlack: {
        unsigned ratio = (unsigned)((transitionTime / _lengthInMs) * 512.0);
        if (ratio < 256) {
          ControlValue firstValue((value.UInt() * (255 - ratio)) >> 8);
          first.MixInput(firstInput, firstValue);
        } else {
          ControlValue secondValue((value.UInt() * (ratio - 256)) >> 8);
          second.MixInput(secondInput, secondValue);
        }
      } break;
      case TransitionType::Erratic: {
        unsigned ratio = (unsigned)((transitionTime / _lengthInMs) *
                                    ControlValue::MaxUInt());
        if (ratio < timing.DrawRandomValue())
          first.MixInput(firstInput, value);
        else
          second.MixInput(secondInput, value);
      }
      case TransitionType::Black:
        break;
      case TransitionType::FadeFromBlack: {
        unsigned ratioValue =
            (unsigned)((transitionTime / _lengthInMs) * 256.0);
        second.MixInput(secondInput, (value.UInt() * ratioValue) >> 8);
      } break;
      case TransitionType::FadeToBlack: {
        unsigned ratioValue =
            255 - (unsigned)((transitionTime / _lengthInMs) * 256.0);
        first.MixInput(secondInput, (value.UInt() * ratioValue) >> 8);
      } break;
    }
  }

 private:
  double _lengthInMs;
  TransitionType _type;
};

}  // namespace glight::theatre

#endif
