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
        _lastBeatValue(0.0),
        _lastBeatTime(0.0),
        _offDelay(2000)  // two seconds
  {}

  virtual Effect::Type GetType() const final override {
    return MusicActivationType;
  }

  unsigned OffDelay() const { return _offDelay; }

  void SetOffDelay(unsigned offDelay) { _offDelay = offDelay; }

 protected:
  virtual void mix(const ControlValue *values,
                   const Timing &timing) final override {
    if (_lastBeatValue != timing.BeatValue()) {
      _lastBeatValue = timing.BeatValue();
      _lastBeatTime = timing.TimeInMS();
    }
    double timePassed = timing.TimeInMS() - _lastBeatTime;
    if (timePassed < _offDelay) {
      for (const std::pair<Controllable *, size_t> &connection : Connections())
        connection.first->MixInput(connection.second, values[0]);
    }
  }

 private:
  double _lastBeatValue;
  double _lastBeatTime;

  unsigned _offDelay;
};

}  // namespace glight::theatre

#endif
