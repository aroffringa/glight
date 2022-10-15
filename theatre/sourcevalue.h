#ifndef THEATRE_SOURCE_VALUE_H_
#define THEATRE_SOURCE_VALUE_H_

#include "presetvalue.h"

namespace glight::theatre {

class Controllable;

class SourceValue {
 public:
  /**
   * Construct a SourceValue that is connected to an input
   * of a controllable.
   */
  SourceValue(Controllable &controllable, size_t input_index)
      : value_(controllable, input_index),
        fade_speed_(0.0),
        target_value_(value_.Value()) {}

  /**
   * Copy constructor that copies the source but associates it with the given
   * controllable.
   */
  SourceValue(const SourceValue &source, Controllable &controllable)
      : value_(source.value_, controllable),
        fade_speed_(0.0),
        target_value_(source.target_value_) {}

  PresetValue &Preset() { return value_; }
  const PresetValue &Preset() const { return value_; }

  bool IsIgnorable() const { return value_.IsIgnorable(); }

  Controllable &GetControllable() const { return value_.GetControllable(); }

  void ApplyFade(double timePassed) {
    unsigned fading_value = value_.Value().UInt();
    if (target_value_ != fading_value) {
      double fadeSpeed = fade_speed_;
      if (fadeSpeed == 0.0) {
        fading_value = target_value_;
      } else {
        unsigned step_size = unsigned(std::min<double>(
            timePassed * fadeSpeed * double(ControlValue::MaxUInt() + 1),
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
      value_.Value().Set(fading_value);
    }
  }

  void Set(unsigned target_value, double fade_speed) {
    target_value_ = target_value;
    fade_speed_ = fade_speed;
    if (fade_speed == 0.0) {
      value_.SetValue(target_value);
    }
  }
  
  double FadeSpeed() const { return fade_speed_; }
  unsigned TargetValue() const { return target_value_; }

 private:
  PresetValue value_;
  double fade_speed_;
  unsigned target_value_;
};

}  // namespace glight::theatre

#endif
