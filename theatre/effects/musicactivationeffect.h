#ifndef MUSIC_ACTIVATION_EFFECT_H
#define MUSIC_ACTIVATION_EFFECT_H

#include "../effect.h"
#include "../timing.h"

#include <string>

namespace glight::theatre {

class MusicActivationEffect final : public Effect {
 public:
  MusicActivationEffect()
      : Effect(1),
        _lastBeatValue{0.0, 0.0},
        _lastBeatTime{0.0, 0.0},
        _offDelay(2000)  // two seconds
  {}

  virtual Effect::Type GetType() const final override {
    return MusicActivationType;
  }

  unsigned OffDelay() const { return _offDelay; }

  void SetOffDelay(unsigned offDelay) { _offDelay = offDelay; }

 protected:
  virtual void mix(const ControlValue *values, const Timing &timing,
                   bool primary) override {
    if (_lastBeatValue[primary] != timing.BeatValue()) {
      _lastBeatValue[primary] = timing.BeatValue();
      _lastBeatTime[primary] = timing.TimeInMS();
    }
    const double timePassed = timing.TimeInMS() - _lastBeatTime[primary];
    if (timePassed < _offDelay) {
      for (const std::pair<Controllable *, size_t> &connection : Connections())
        connection.first->MixInput(connection.second, values[0]);
    }
  }

 private:
  double _lastBeatValue[2];
  double _lastBeatTime[2];

  unsigned _offDelay;
};

}  // namespace glight::theatre

#endif
