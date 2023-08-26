#ifndef THEATRE_SOURCE_VALUE_H_
#define THEATRE_SOURCE_VALUE_H_

#include "controlvalue.h"
#include "input.h"

#include <sigc++/signal.h>

namespace glight::theatre {

class Controllable;

class SingleSourceValue {
 public:
  SingleSourceValue() = default;
  SingleSourceValue(const SingleSourceValue& source) = default;

  SingleSourceValue& operator=(const SingleSourceValue& source) = default;

  bool IsIgnorable() const { return !value_; }

  void ApplyFade(double time_passed) {
    unsigned fading_value = value_.UInt();
    if (target_value_ != fading_value) {
      double fadeSpeed = fade_speed_;
      if (fadeSpeed == 0.0) {
        fading_value = target_value_;
      } else {
        unsigned step_size = unsigned(std::min<double>(
            time_passed * fadeSpeed * double(ControlValue::MaxUInt() + 1),
            double(ControlValue::MaxUInt() + 1)));
        if (target_value_ > fading_value) {
          if (fading_value + step_size > target_value_)
            fading_value = target_value_;
          else
            fading_value += step_size;
        } else {
          if (target_value_ + step_size > fading_value)
            fading_value = target_value_;
          else
            fading_value -= step_size;
        }
      }
      value_ = ControlValue(fading_value);
    }
  }

  /**
   * Starts a fade towards the given target value. A fade
   * of zero can be used to immediately move to the target
   * value, but @ref Set(unsigned) can also be used in that
   * case.
   */
  void Set(unsigned target_value, double fade_speed) {
    target_value_ = target_value;
    fade_speed_ = fade_speed;
    if (fade_speed == 0.0) {
      value_ = ControlValue(target_value);
    }
  }

  /**
   * Set the current and target value directly (without
   * fade) to the given value and resets the fade. This
   * is equivalent with Set(value, 0.0).
   */
  void Set(unsigned immediate_target_value) {
    target_value_ = immediate_target_value;
    fade_speed_ = 0.0;
    value_ = ControlValue(immediate_target_value);
  }

  /**
   * Sets the current value of the source value. Note that the value
   * will fade towards the target value, so this should not be used
   * when the source value needs to be changed directly.
   */
  void SetValue(const ControlValue& value) { value_ = value; }
  const ControlValue& Value() const { return value_; }

  void SetFadeSpeed(double fade_speed) { fade_speed_ = fade_speed; }
  double FadeSpeed() const { return fade_speed_; }

  void SetTargetValue(unsigned target_value) { target_value_ = target_value; }
  unsigned TargetValue() const { return target_value_; }

 private:
  ControlValue value_ = ControlValue(0u);
  double fade_speed_ = 0.0;
  unsigned target_value_ = 0;
};

/**
 * A SourceValue represents a node to which a fader can be connected.
 */
class SourceValue {
 public:
  /**
   * Construct a SourceValue that is connected to an input
   * of a controllable.
   */
  SourceValue(Controllable& controllable, size_t input_index)
      : input_(controllable, input_index), a_(), b_(), cross_fader_() {}
  ~SourceValue() { signal_delete_(); }

  SingleSourceValue& A() { return a_; }
  const SingleSourceValue& A() const { return a_; }

  SingleSourceValue& B() { return b_; }
  const SingleSourceValue& B() const { return b_; }

  SingleSourceValue& CrossFader() { return cross_fader_; }
  const SingleSourceValue& CrossFader() const { return cross_fader_; }

  const Controllable& GetControllable() const {
    return *input_.GetControllable();
  }
  Controllable& GetControllable() { return *input_.GetControllable(); }
  size_t InputIndex() const { return input_.InputIndex(); }
  std::string Name() const;
  sigc::signal<void()>& SignalDelete() { return signal_delete_; }

  void Reconnect(Controllable& controllable, size_t input_index) {
    input_ = Input(controllable, input_index);
  }
  void ApplyFade(double time_passed) {
    a_.ApplyFade(time_passed);
    b_.ApplyFade(time_passed);
    cross_fader_.ApplyFade(time_passed);
  }
  unsigned PrimaryValue() const {
    return (a_.Value() * Invert(cross_fader_.Value())).UInt() +
           (b_.Value() * cross_fader_.Value()).UInt();
  }
  unsigned SecondaryValue() const {
    return (b_.Value() * Invert(cross_fader_.Value())).UInt() +
           (a_.Value() * cross_fader_.Value()).UInt();
  }
  /**
   * Swap a and b and flip the cross fader.
   * This won't change the mix output.
   */
  void Swap() {
    std::swap(a_, b_);
    cross_fader_.SetValue(Invert(cross_fader_.Value()));
    cross_fader_.SetTargetValue(
        ControlValue::Invert(cross_fader_.TargetValue()));
  }

 private:
  Input input_;
  SingleSourceValue a_;
  SingleSourceValue b_;
  SingleSourceValue cross_fader_;
  sigc::signal<void()> signal_delete_;
};

}  // namespace glight::theatre

#include "controllable.h"

inline std::string glight::theatre::SourceValue::Name() const {
  return GetControllable().InputName(InputIndex());
}

#endif
