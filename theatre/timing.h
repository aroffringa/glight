#ifndef THEATRE_TIMING_H_
#define THEATRE_TIMING_H_

#include <random>

namespace glight::theatre {

class Timing {
 public:
  Timing()
      : _timeInMs(0),
        _timestepNumber(0),
        _beatValue(0),
        _audioLevel(0),
        _randomValue(0),
        _rng() {}

  Timing(double timeInMS, unsigned timestepNumber, double beatValue,
         unsigned audioLevel, unsigned randomValue)
      : _timeInMs(timeInMS),
        _timestepNumber(timestepNumber),
        _beatValue(beatValue),
        _audioLevel(audioLevel),
        _randomValue(randomValue),
        _rng(randomValue) {}

  double TimeInMS() const { return _timeInMs; }
  double BeatValue() const { return _beatValue; }
  unsigned TimestepNumber() const { return _timestepNumber; }
  unsigned AudioLevel() const { return _audioLevel; }

  unsigned TimestepRandomValue() const { return _randomValue; }
  unsigned DrawRandomValue() const {
    return std::uniform_int_distribution<unsigned>(
        0, ControlValue::MaxUInt() + 1)(_rng);
  }
  unsigned DrawRandomValue(size_t maxValue) const {
    return std::uniform_int_distribution<unsigned>(0, maxValue)(_rng);
  }
  double DrawGaussianValue() const {
    return std::normal_distribution<double>(0.0, 1.0)(_rng);
  }

  std::mt19937 &RNG() const { return _rng; }

 private:
  double _timeInMs;
  unsigned _timestepNumber;
  double _beatValue;
  unsigned _audioLevel, _randomValue;
  mutable std::mt19937 _rng;
};

}  // namespace glight::theatre

#endif
