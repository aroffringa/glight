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
  Full,
  FadeFromBlack,
  FadeToBlack,
  FadeFromFull,
  FadeToFull
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
    case TransitionType::Full:
      return "full";
    case TransitionType::FadeFromBlack:
      return "fade_from_black";
    case TransitionType::FadeToBlack:
      return "fade_to_black";
    case TransitionType::FadeFromFull:
      return "fade_from_full";
    case TransitionType::FadeToFull:
      return "fade_to_full";
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
  else if (str == "full")
    return TransitionType::Full;
  else if (str == "fade_from_black")
    return TransitionType::FadeFromBlack;
  else if (str == "fade_to_black")
    return TransitionType::FadeToBlack;
  else if (str == "fade_from_full")
    return TransitionType::FadeFromFull;
  else if (str == "fade_to_full")
    return TransitionType::FadeToFull;
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
      : length_in_ms_(length_in_ms), type_(type) {}

  constexpr TransitionType Type() const { return type_; }
  void SetType(TransitionType type) { type_ = type; }

  constexpr double LengthInMs() const { return length_in_ms_; }
  void SetLengthInMs(double length) { length_in_ms_ = length; }

  /**
   * Calculates the control value to transition towards. This implies
   * that for a transition from A to B, this function returns
   * the value of B at a given time.
   * @param transition_time value between 0 and _lengthInMS.
   * @param timing used for randomness, etc.
   */
  ControlValue InValue(double transition_time, const Timing &timing) const;

  /**
   * Calculate the control value that's being transitioned from. This implies
   * that for a transition from A to B, this function returns the value of
   * A at a given time.
   * @param transition_time value between 0 and _lengthInMS.
   * @param timing used for randomness, etc.
   */
  ControlValue OutValue(double transition_time, const Timing &timing) const;

  /**
   * Mix two controllables that are transitioning.
   * @param transition_time value between 0 and _lengthInMS.
   * @param timing used for randomness, etc.
   */
  void Mix(Controllable &first, size_t first_input, Controllable &second,
           size_t second_input, double transition_time,
           const ControlValue &value, const Timing &timing) const;

 private:
  double length_in_ms_ = 250.0;
  TransitionType type_ = TransitionType::Fade;
};

}  // namespace glight::theatre

#endif
