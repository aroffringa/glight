#include "transition.h"

namespace glight::theatre {

ControlValue Transition::InValue(double transition_time,
                                 const Timing &timing) const {
  switch (type_) {
    case TransitionType::None:
      if (transition_time * 2.0 <= length_in_ms_)
        return ControlValue::Zero();
      else
        return ControlValue::Max();
    case TransitionType::Fade:
    case TransitionType::FadeFromBlack:
    case TransitionType::FadeToFull:
      return ControlValue::FromRatio(transition_time / length_in_ms_);
    case TransitionType::FadeThroughBlack: {
      const double ratio = transition_time / length_in_ms_;
      if (ratio >= 0.5) {
        return ControlValue::FromRatio(ratio * 2.0 - 1.0);
      } else {
        return ControlValue::Zero();
      }
    }
    case TransitionType::FadeThroughFull: {
      const double ratio = transition_time / length_in_ms_;
      if (ratio >= 0.5) {
        return ControlValue::Max();
      } else {
        return ControlValue::FromRatio(ratio * 2.0);
      }
    }
    case TransitionType::GlowFade: {
      constexpr double stage_split = 0.25;
      double transition_point = transition_time / length_in_ms_;
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
          static_cast<unsigned>((transition_time / length_in_ms_) * 5.0);
      return ControlValue(value * ControlValue::MaxUInt() / 5);
    }
    case TransitionType::ConstantAcceleration: {
      const double ratio = transition_time / length_in_ms_;
      if (ratio <= 0.5)
        return ControlValue::FromRatio(ratio * ratio * 2.0);
      else
        return ControlValue::FromRatio(1.0 -
                                       (ratio - 1.0) * (ratio - 1.0) * 2.0);
    }
    case TransitionType::Random: {
      const unsigned ratio =
          (unsigned)((transition_time / length_in_ms_) * 256);
      const unsigned upper_bound = std::min(255u, ratio * 2u);
      const unsigned lower_bound = std::max(ratio * 2u, 256u) - 256u;
      const unsigned value =
          timing.DrawRandomValue(upper_bound - lower_bound) + lower_bound;
      return ControlValue(value << 16);
    }
    case TransitionType::Erratic: {
      unsigned ratio = (unsigned)((transition_time / length_in_ms_) *
                                  ControlValue::MaxUInt());
      if (ratio >= timing.DrawRandomValue())
        return ControlValue::Max();
      else
        return ControlValue::Zero();
    }
    case TransitionType::SlowStrobe:
      return timing.TimestepNumber() % 8 == 0 ? ControlValue::Max()
                                              : ControlValue::Zero();
    case TransitionType::FastStrobe:
      return timing.TimestepNumber() % 2 == 0 ? ControlValue::Max()
                                              : ControlValue::Zero();
    case TransitionType::StrobeAB:
      return transition_time * 2.0 < length_in_ms_ &&
                     timing.TimestepNumber() % 2 == 0
                 ? ControlValue::Max()
                 : ControlValue::Zero();
    case TransitionType::Black:
      return ControlValue::Zero();
    case TransitionType::Full:
    case TransitionType::FadeFromFull:
      return ControlValue::Max();
    case TransitionType::FadeToBlack: {
      const unsigned ratio = (unsigned)((transition_time / length_in_ms_) *
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
ControlValue Transition::OutValue(double transition_time,
                                  const Timing &timing) const {
  switch (type_) {
    case TransitionType::None:
      if (transition_time * 2.0 > length_in_ms_)
        return ControlValue::Zero();
      else
        return ControlValue::Max();
    case TransitionType::Fade:
    case TransitionType::FadeFromFull: {
      const double ratio = 1.0 - (transition_time / length_in_ms_);
      return ControlValue::FromRatio(ratio);
    }
    case TransitionType::FadeThroughBlack: {
      const double ratio = (transition_time / length_in_ms_) * 2.0;
      return ratio < 1.0 ? ControlValue::FromRatio(1.0 - ratio)
                         : ControlValue::Zero();
    }
    case TransitionType::FadeThroughFull: {
      const double ratio = (transition_time / length_in_ms_) * 2.0;
      if (ratio < 1.0) {
        return ControlValue::Max();
      } else {
        return ControlValue::FromRatio(2.0 - ratio);
      }
    }
    case TransitionType::GlowFade: {
      constexpr double stage_split = 0.25;
      constexpr double glow_level = 0.1;
      double transition_point = transition_time / length_in_ms_;
      if (transition_point < stage_split) {
        transition_point /= stage_split;
        const double x =
            (1.0 - transition_point) * (1.0 - glow_level) + glow_level;
        return ControlValue::FromRatio(x);
      } else {
        transition_point =
            (transition_point - stage_split) / (1.0 - stage_split);
        return ControlValue::FromRatio((1.0 - transition_point) * glow_level);
      }
    }
    case TransitionType::Stepped: {
      double ratio =
          1.0 - std::floor(4.999 * transition_time / length_in_ms_) / 5.0;
      return ControlValue::FromRatio(ratio);
    }
    case TransitionType::ConstantAcceleration: {
      const double ratio = transition_time / length_in_ms_;
      if (ratio <= 0.5)
        return ControlValue::FromRatio(1.0 - ratio * ratio * 2.0);
      else
        return ControlValue::FromRatio((ratio - 1.0) * (ratio - 1.0) * 2.0);
    }
    case TransitionType::Random: {
      const unsigned ratio =
          (unsigned)((transition_time / length_in_ms_) * 256);
      const unsigned upper_bound = std::min(255u, ratio * 2u);
      const unsigned lower_bound = std::max(ratio * 2u, 256u) - 256u;
      const unsigned value =
          255 -
          (timing.DrawRandomValue(upper_bound - lower_bound) + lower_bound);
      return ControlValue(value << 16);
    }
    case TransitionType::Erratic: {
      unsigned ratio = (unsigned)((transition_time / length_in_ms_) *
                                  ControlValue::MaxUInt());
      if (ratio < timing.DrawRandomValue())
        return ControlValue::Max();
      else
        return ControlValue::Zero();
    }
    case TransitionType::SlowStrobe:
      return timing.TimestepNumber() % 8 + 4 == 0 ? ControlValue::Max()
                                                  : ControlValue::Zero();
    case TransitionType::FastStrobe:
      return timing.TimestepNumber() % 2 == 1 ? ControlValue::Max()
                                              : ControlValue::Zero();
    case TransitionType::StrobeAB:
      return transition_time * 2.0 >= length_in_ms_ &&
                     timing.TimestepNumber() % 2 == 0
                 ? ControlValue::Max()
                 : ControlValue::Zero();
    case TransitionType::Black:
    case TransitionType::FadeFromBlack: {
      return ControlValue::Zero();
    }
    case TransitionType::FadeToBlack: {
      const double ratio = 1.0 - (transition_time / length_in_ms_);
      return ControlValue::FromRatio(ratio);
    }
    case TransitionType::Full:
    case TransitionType::FadeToFull:
      return ControlValue::Max();
  }
  assert(false);
  return ControlValue::Zero();
}

/**
 * @param transitionTime value between 0 and _lengthInMS.
 */
void Transition::Mix(Controllable &first, size_t first_input,
                     Controllable &second, size_t second_input,
                     double transition_time, const ControlValue &value,
                     const Timing &timing) const {
  const double ratio = std::clamp(transition_time / length_in_ms_, 0.0, 1.0);
  switch (type_) {
    case TransitionType::None:
      if (transition_time * 2.0 <= length_in_ms_)
        first.MixInput(first_input, value);
      else
        second.MixInput(second_input, value);
      break;
    case TransitionType::Fade: {
      first.MixInput(first_input, value * (1.0 - ratio));
      second.MixInput(second_input, value * ratio);
    } break;
    case TransitionType::FadeThroughBlack: {
      const unsigned scaled_ratio = (unsigned)(ratio * (65536 * 2.0));
      if (scaled_ratio < 65536) {
        ControlValue firstValue(
            ((value.UInt() >> 8) * (65535 - scaled_ratio)) >> 8);
        first.MixInput(first_input, firstValue);
      } else {
        ControlValue secondValue(
            ((value.UInt() >> 8) * (scaled_ratio - 65536)) >> 8);
        second.MixInput(second_input, secondValue);
      }
    } break;
    case TransitionType::FadeThroughFull: {
      const unsigned scaled_ratio = (unsigned)(ratio * (65536 * 2.0));
      if (scaled_ratio < 65536) {
        first.MixInput(first_input, value);
        const ControlValue secondValue(((value.UInt() >> 8) * scaled_ratio) >>
                                       8);
        second.MixInput(second_input, secondValue);
      } else {
        const ControlValue firstValue(
            ((value.UInt() >> 8) * (512 - scaled_ratio)) >> 8);
        first.MixInput(first_input, firstValue);
        second.MixInput(second_input, value);
      }
    } break;
    case TransitionType::GlowFade: {
      // This fade is divided in 2 stages:
      // - Quickly fade up B, quickly but partially fade down A
      // - B is full on, slowly finish fading down A ("glow")
      constexpr double stage_split = 0.25;
      constexpr double glow_level = 0.1;
      double transition_point = ratio;
      if (transition_point < stage_split) {
        transition_point /= stage_split;
        const double a =
            (1.0 - transition_point) * (1.0 - glow_level) + glow_level;
        first.MixInput(first_input, ControlValue(value.UInt() * a));
        second.MixInput(second_input,
                        ControlValue(value.UInt() * transition_point));
      } else {
        transition_point =
            (transition_point - stage_split) / (1.0 - stage_split);
        const double a = (1.0 - transition_point) * glow_level;
        first.MixInput(first_input, ControlValue(value.UInt() * a));
        second.MixInput(second_input, value);
      }
    } break;
    case TransitionType::Stepped: {
      unsigned secondRatioValue = (unsigned)(ratio * 256.0);
      secondRatioValue = (secondRatioValue / 51) * 51;
      const unsigned firstRatioValue = 255 - secondRatioValue;
      first.MixInput(first_input,
                     ControlValue((value.UInt() * firstRatioValue) >> 8));
      second.MixInput(second_input,
                      ControlValue((value.UInt() * secondRatioValue) >> 8));
    } break;
    case TransitionType::ConstantAcceleration: {
      const double fade_value = (ratio <= 0.5)
                                    ? ratio * ratio * 2.0
                                    : 1.0 - (ratio - 1.0) * (ratio - 1.0) * 2.0;
      unsigned secondRatioValue = (unsigned)(fade_value * 65536.0);
      const unsigned firstRatioValue = 65535 - secondRatioValue;
      first.MixInput(
          first_input,
          ControlValue(((value.UInt() >> 8) * firstRatioValue) >> 8));
      second.MixInput(
          second_input,
          ControlValue(((value.UInt() >> 8) * secondRatioValue) >> 8));
    } break;
    case TransitionType::Random: {
      const unsigned scaled_ratio = (unsigned)(ratio * 256);
      const unsigned upper_bound = std::min(255u, scaled_ratio * 2u);
      const unsigned lower_bound = std::max(scaled_ratio * 2u, 256u) - 256u;
      const unsigned secondRatioValue =
          timing.DrawRandomValue(upper_bound - lower_bound) + lower_bound;
      const unsigned firstRatioValue = 255 - secondRatioValue;
      first.MixInput(first_input,
                     ControlValue((value.UInt() * firstRatioValue) >> 8));
      second.MixInput(second_input,
                      ControlValue((value.UInt() * secondRatioValue) >> 8));
    } break;
    case TransitionType::Erratic: {
      unsigned scaled_ratio = (unsigned)(ratio * ControlValue::MaxUInt());
      if (scaled_ratio < timing.DrawRandomValue())
        first.MixInput(first_input, value);
      else
        second.MixInput(second_input, value);
    } break;
    case TransitionType::SlowStrobe:
      if (timing.TimestepNumber() % 8 == 0)
        first.MixInput(first_input, value);
      else if (timing.TimestepNumber() % 8 == 4)
        second.MixInput(second_input, value);
      break;
    case TransitionType::FastStrobe:
      if (timing.TimestepNumber() % 2 == 0)
        first.MixInput(first_input, value);
      else
        second.MixInput(second_input, value);
      break;
    case TransitionType::StrobeAB: {
      if (timing.TimestepNumber() % 2 == 0) {
        if (transition_time * 2.0 < length_in_ms_)
          first.MixInput(first_input, value);
        else
          second.MixInput(second_input, value);
      }
    } break;
    case TransitionType::Black:
      break;
    case TransitionType::Full:
      first.MixInput(first_input, value);
      second.MixInput(second_input, value);
      break;
    case TransitionType::FadeFromBlack: {
      unsigned ratioValue = (unsigned)(ratio * 65536.0);
      second.MixInput(second_input,
                      ControlValue(((value.UInt() >> 8) * ratioValue) >> 8));
    } break;
    case TransitionType::FadeToBlack: {
      unsigned ratioValue = 65535 - (unsigned)(ratio * 65536.0);
      first.MixInput(second_input,
                     ControlValue(((value.UInt() >> 8) * ratioValue) >> 8));
    } break;
    case TransitionType::FadeFromFull: {
      const unsigned ratio_value = 65535 - (unsigned)(ratio * 65536.0);
      first.MixInput(first_input,
                     ControlValue(((value.UInt() >> 8) * ratio_value) >> 8));
      second.MixInput(second_input, value);
    } break;
    case TransitionType::FadeToFull: {
      unsigned ratio_value = (unsigned)(ratio * 65536.0);
      first.MixInput(first_input, value);
      second.MixInput(second_input,
                      ControlValue(((value.UInt() >> 8) * ratio_value) >> 8));
    } break;
  }
}

}  // namespace glight::theatre
