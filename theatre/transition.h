#ifndef THEATRE_TRANSITION_H_
#define THEATRE_TRANSITION_H_

#include "presetcollection.h"
#include "timing.h"

#include <cassert>

namespace glight::theatre {

enum class TransitionType {
  None,
  Fade,
  FadeThroughBlack,
  FadeThroughFull,
  GlowFade,
  Random,
  Stepped,
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
    case TransitionType::FadeThroughFull:
      return "fade_through_full";
    case TransitionType::GlowFade:
      return "glow_fade";
    case TransitionType::Random:
      return "random";
    case TransitionType::Stepped:
      return "stepped";
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
  else if (str == "fade_through_full")
    return TransitionType::FadeThroughFull;
  else if (str == "glow_fade")
    return TransitionType::GlowFade;
  else if (str == "random")
    return TransitionType::Random;
  else if (str == "stepped")
    return TransitionType::Stepped;
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
  constexpr Transition() noexcept = default;
  constexpr ~Transition() noexcept = default;

  constexpr Transition(double length_in_ms, TransitionType type) noexcept
      : _lengthInMs(length_in_ms), _type(type) {}

  constexpr TransitionType Type() const { return _type; }
  void SetType(TransitionType type) { _type = type; }

  constexpr double LengthInMs() const { return _lengthInMs; }
  void SetLengthInMs(double length) { _lengthInMs = length; }

  /**
   * Calculates the control value transition towards. This implies
   * that for a transition from A to B, this function returns
   * the value of B at a given time.
   */
  ControlValue TransitionValue(double transition_time,
                               const Timing &timing) const {
    switch (_type) {
      case TransitionType::None:
        if (transition_time * 2.0 <= _lengthInMs)
          return ControlValue::Zero();
        else
          return ControlValue::Max();
      case TransitionType::FadeFromBlack:
      case TransitionType::Fade: {
        const unsigned ratio =
            (unsigned)((transition_time / _lengthInMs) *
                       static_cast<double>(ControlValue::MaxUInt()));
        return ControlValue(ratio);
      }
      case TransitionType::FadeThroughFull:
      case TransitionType::FadeThroughBlack: {
        if (transition_time >= 0.5) {
          const unsigned ratio = (unsigned)((transition_time / _lengthInMs) *
                                            ControlValue::MaxUInt() * 2);
          return ControlValue(ratio - ControlValue::MaxUInt());
        } else {
          return ControlValue::Zero();
        }
      }
      case TransitionType::GlowFade: {
        constexpr double stage_split = 0.25;
        double transition_point = transition_time / _lengthInMs;
        if (transition_point < stage_split) {
          transition_point /= stage_split;
          return ControlValue(ControlValue::Max() * transition_point);
        } else {
          transition_point =
              (transition_point - stage_split) / (1.0 - stage_split);
          return ControlValue::Max();
        }
      }
      case TransitionType::Stepped: {
        const unsigned value =
            static_cast<unsigned>((transition_time / _lengthInMs) * 5.0);
        return ControlValue(value * ControlValue::MaxUInt() / 5);
      }
      case TransitionType::Random: {
        const unsigned ratio =
            (unsigned)((transition_time / _lengthInMs) * 256);
        const unsigned upper_bound = std::min(255u, ratio * 2u);
        const unsigned lower_bound = std::max(ratio * 2u, 256u) - 256u;
        const unsigned value =
            timing.DrawRandomValue(upper_bound - lower_bound) + lower_bound;
        return ControlValue(value << 16);
      }
      case TransitionType::Erratic: {
        unsigned ratio = (unsigned)((transition_time / _lengthInMs) *
                                    ControlValue::MaxUInt());
        if (ratio >= timing.DrawRandomValue())
          return ControlValue::Max();
        else
          return ControlValue::Zero();
      }
      case TransitionType::Black:
        return ControlValue::Zero();
      case TransitionType::FadeToBlack: {
        const unsigned ratio = (unsigned)((transition_time / _lengthInMs) *
                                          ControlValue::MaxUInt());
        return ControlValue(ControlValue::MaxUInt() - ratio);
      }
    }
    assert(false);
    return ControlValue::Zero();
  }

  /**
   * @param transitionTime value between 0 and _lengthInMS.
   */
  void Mix(Controllable &first, size_t firstInput, Controllable &second,
           size_t secondInput, double transition_time,
           const ControlValue &value, const Timing &timing) const {
    switch (_type) {
      case TransitionType::None:
        if (transition_time * 2.0 <= _lengthInMs)
          first.MixInput(firstInput, value);
        else
          second.MixInput(secondInput, value);
        break;
      case TransitionType::Fade: {
        const unsigned secondRatioValue =
            (unsigned)((transition_time / _lengthInMs) * 256.0);
        const unsigned firstRatioValue = 255 - secondRatioValue;
        first.MixInput(firstInput,
                       ControlValue((value.UInt() * firstRatioValue) >> 8));
        second.MixInput(secondInput,
                        ControlValue((value.UInt() * secondRatioValue) >> 8));
      } break;
      case TransitionType::FadeThroughBlack: {
        const unsigned ratio =
            (unsigned)((transition_time / _lengthInMs) * 512.0);
        if (ratio < 256) {
          ControlValue firstValue((value.UInt() * (255 - ratio)) >> 8);
          first.MixInput(firstInput, firstValue);
        } else {
          ControlValue secondValue((value.UInt() * (ratio - 256)) >> 8);
          second.MixInput(secondInput, secondValue);
        }
      } break;
      case TransitionType::FadeThroughFull: {
        const unsigned ratio =
            (unsigned)((transition_time / _lengthInMs) * 512.0);
        if (ratio < 256) {
          first.MixInput(firstInput, value);
          const ControlValue secondValue((value.UInt() * ratio) >> 8);
          second.MixInput(secondInput, secondValue);
        } else {
          const ControlValue firstValue((value.UInt() * (512 - ratio)) >> 8);
          first.MixInput(firstInput, firstValue);
          second.MixInput(secondInput, value);
        }
      } break;
      case TransitionType::GlowFade: {
        // This fade is divided in 2 stages:
        // - Quickly fade up B, quickly but partially fade down A
        // - B is full on, slowly finish fading down A ("glow")
        constexpr double stage_split = 0.25;
        constexpr double glow_level = 0.1;
        double transition_point = transition_time / _lengthInMs;
        if (transition_point < stage_split) {
          transition_point /= stage_split;
          const double a =
              (1.0 - transition_point) * (1.0 - glow_level) + glow_level;
          first.MixInput(firstInput, ControlValue(value.UInt() * a));
          second.MixInput(secondInput,
                          ControlValue(value.UInt() * transition_point));
        } else {
          transition_point =
              (transition_point - stage_split) / (1.0 - stage_split);
          const double a = (1.0 - transition_point) * glow_level;
          first.MixInput(firstInput, ControlValue(value.UInt() * a));
          second.MixInput(secondInput, value);
        }
      } break;
      case TransitionType::Stepped: {
        unsigned secondRatioValue =
            (unsigned)((transition_time / _lengthInMs) * 256.0);
        secondRatioValue = (secondRatioValue / 51) * 51;
        const unsigned firstRatioValue = 255 - secondRatioValue;
        first.MixInput(firstInput,
                       ControlValue((value.UInt() * firstRatioValue) >> 8));
        second.MixInput(secondInput,
                        ControlValue((value.UInt() * secondRatioValue) >> 8));
      } break;
      case TransitionType::Random: {
        const unsigned ratio =
            (unsigned)((transition_time / _lengthInMs) * 256);
        const unsigned upper_bound = std::min(255u, ratio * 2u);
        const unsigned lower_bound = std::max(ratio * 2u, 256u) - 256u;
        const unsigned secondRatioValue =
            timing.DrawRandomValue(upper_bound - lower_bound) + lower_bound;
        const unsigned firstRatioValue = 255 - secondRatioValue;
        first.MixInput(firstInput,
                       ControlValue((value.UInt() * firstRatioValue) >> 8));
        second.MixInput(secondInput,
                        ControlValue((value.UInt() * secondRatioValue) >> 8));
      } break;
      case TransitionType::Erratic: {
        unsigned ratio = (unsigned)((transition_time / _lengthInMs) *
                                    ControlValue::MaxUInt());
        if (ratio < timing.DrawRandomValue())
          first.MixInput(firstInput, value);
        else
          second.MixInput(secondInput, value);
      } break;
      case TransitionType::Black:
        break;
      case TransitionType::FadeFromBlack: {
        unsigned ratioValue =
            (unsigned)((transition_time / _lengthInMs) * 256.0);
        second.MixInput(secondInput,
                        ControlValue((value.UInt() * ratioValue) >> 8));
      } break;
      case TransitionType::FadeToBlack: {
        unsigned ratioValue =
            255 - (unsigned)((transition_time / _lengthInMs) * 256.0);
        first.MixInput(secondInput,
                       ControlValue((value.UInt() * ratioValue) >> 8));
      } break;
    }
  }

 private:
  double _lengthInMs = 250.0;
  TransitionType _type = TransitionType::Fade;
};

}  // namespace glight::theatre

#endif
