#ifndef THEATRE_TIMING_H_
#define THEATRE_TIMING_H_

#include <random>

namespace glight::theatre {

class Timing {
 public:
  Timing() noexcept = default;

  Timing(double timeInMS, unsigned timestepNumber, double beatValue,
         unsigned audioLevel, unsigned randomValue) noexcept
      : time_in_ms_(timeInMS),
        timestep_number_(timestepNumber),
        beat_value_(beatValue),
        audio_level_(audioLevel),
        random_value_(randomValue),
        rng_(randomValue) {}

  static Timing MakeForDebug(double time_in_ms) {
    Timing timing;
    timing.time_in_ms_ = time_in_ms;
    return timing;
  }

  double TimeInMS() const { return time_in_ms_; }
  double BeatValue() const { return beat_value_; }
  unsigned TimestepNumber() const { return timestep_number_; }
  unsigned AudioLevel() const { return audio_level_; }

  unsigned TimestepRandomValue() const { return random_value_; }
  unsigned DrawRandomValue() const {
    return std::uniform_int_distribution<unsigned>(
        0, ControlValue::MaxUInt() + 1)(rng_);
  }
  unsigned DrawRandomValue(size_t maxValue) const {
    return std::uniform_int_distribution<unsigned>(0, maxValue)(rng_);
  }
  double DrawGaussianValue() const {
    return std::normal_distribution<double>(0.0, 1.0)(rng_);
  }

  std::mt19937 &RNG() const { return rng_; }

 private:
  double time_in_ms_ = 0.0;
  unsigned timestep_number_ = 0;
  double beat_value_ = 0.0;
  unsigned audio_level_ = 0;
  unsigned random_value_ = 0;
  mutable std::mt19937 rng_;
};

}  // namespace glight::theatre

#endif
