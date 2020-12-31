#ifndef PULSE_EFFECT_H
#define PULSE_EFFECT_H

#include "../effect.h"
#include "../timing.h"
#include "../transition.h"

class PulseEffect : public Effect {
 public:
  PulseEffect()
      : Effect(1),
        _isActive(false),
        _startTime(0),
        _repeat(false),
        _attack(300),
        _hold(200),
        _release(300),
        _sleep(200) {}

  virtual Effect::Type GetType() const final override { return PulseType; }

  unsigned Attack() const { return _attack; }
  void SetAttack(unsigned attack) { _attack = attack; }

  unsigned Hold() const { return _hold; }
  void SetHold(unsigned hold) { _hold = hold; }

  unsigned Release() const { return _release; }
  void SetRelease(unsigned release) { _release = release; }

  unsigned Sleep() const { return _sleep; }
  void SetSleep(unsigned sleep) { _sleep = sleep; }

  bool Repeat() const { return _repeat; }
  void SetRepeat(bool repeat) { _repeat = repeat; }

 protected:
  virtual void mix(const ControlValue *values,
                   const Timing &timing) final override {
    if (values[0].UInt() == 0) {
      _isActive = false;
    } else {
      if (!_isActive) {
        _startTime = timing.TimeInMS();
        _isActive = true;
      }
      double pos = timing.TimeInMS() - _startTime;
      size_t cycleDuration = _attack + _hold + _release + _sleep;
      if (_repeat || pos < cycleDuration) {
        pos = std::fmod(pos, cycleDuration);
        bool handled = false;

        if (_attack != 0) {
          if (pos < _attack) {
            // Fade in
            unsigned ratio = (unsigned)((pos / double(_attack)) * 256.0);
            setConnectedInputs(ControlValue((values[0].UInt() * ratio) >> 8));
            handled = true;
          } else
            pos -= _attack;
        }

        if (_hold != 0 && !handled) {
          if (pos < _hold) {
            setConnectedInputs(ControlValue(values[0].UInt()));
            handled = true;
          } else
            pos -= _hold;
        }

        if (_release != 0 && !handled) {
          if (pos < _release) {
            // Fade out
            unsigned ratio = 255 - (unsigned)((pos / double(_release)) * 256.0);
            setConnectedInputs(ControlValue((values[0].UInt() * ratio) >> 8));
            handled = true;
          } else
            pos -= _hold;
        }
      }
    }
  }

 private:
  bool _isActive;
  double _startTime;

  bool _repeat;
  // all in ms.
  unsigned _attack, _hold, _release, _sleep;
};

#endif
