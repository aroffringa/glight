#ifndef AUDIO_LEVEL_EFFECT_H
#define AUDIO_LEVEL_EFFECT_H

#include "../effect.h"
#include "../timing.h"

#include <string>

namespace glight::theatre {

class AudioLevelEffect final : public Effect {
 public:
  AudioLevelEffect()
      : Effect(1),
        _lastValue{0, 0},
        _lastTime{0.0, 0.0},
        _decaySpeed(((1 << 24) - 1) / 300)  // fully decay in 300 ms
  {}

  virtual Effect::Type GetType() const final override { return AudioLevelType; }

  unsigned DecaySpeed() const { return _decaySpeed; }

  void SetDecaySpeed(unsigned decaySpeed) { _decaySpeed = decaySpeed; }

 protected:
  virtual void mix(const ControlValue *values, const Timing &timing,
                   bool primary) override {
    unsigned audioLevel = (unsigned(timing.AudioLevel()) << 8);
    double timePassed = timing.TimeInMS() - _lastTime[primary];
    _lastTime[primary] = timing.TimeInMS();
    unsigned decay =
        unsigned(std::min<double>(timePassed * _decaySpeed, (1 << 24) - 1));
    if (_lastValue[primary] < decay)
      _lastValue[primary] = 0;
    else
      _lastValue[primary] -= decay;
    _lastValue[primary] = std::max(_lastValue[primary], audioLevel);

    unsigned v = ControlValue::Mix(_lastValue[primary], values[0].UInt(),
                                   MixStyle::Multiply);
    ControlValue audioLevelCV(v);
    for (const std::pair<Controllable *, size_t> &connection : Connections()) {
      connection.first->MixInput(connection.second, audioLevelCV);
    }
  }

 private:
  unsigned _lastValue[2];
  double _lastTime[2];

  // 2^24-1 means fully fade down in one second
  unsigned _decaySpeed;
};

}  // namespace glight::theatre

#endif
