#ifndef FADE_EFFECT_H
#define FADE_EFFECT_H

#include "../effect.h"
#include "../timing.h"

namespace glight::theatre {

class FadeEffect final : public Effect {
 public:
  FadeEffect()
      : Effect(1),
        _fadingValue{0, 0},
        _previousTime{0.0, 0.0},
        _sustainTimer{0.0, 0.0},
        _fadeUpSpeed(1.0),
        _fadeDownSpeed(1.0),
        _sustain(0.0) {}

  virtual EffectType GetType() const override { return EffectType::Fade; }

  double FadeUpDuration() const {
    return _fadeUpSpeed == 0.0 ? 0.0 : 1.0e3 / _fadeUpSpeed;
  }
  void SetFadeUpDuration(double durationMS) {
    _fadeUpSpeed = durationMS == 0.0 ? 0.0 : 1.0e3 / durationMS;
  }

  double FadeDownDuration() const {
    return _fadeDownSpeed == 0.0 ? 0.0 : 1.0e3 / _fadeDownSpeed;
  }
  void SetFadeDownDuration(double durationMS) {
    _fadeDownSpeed = durationMS == 0.0 ? 0.0 : 1.0e3 / durationMS;
  }

  double Sustain() const { return _sustain * 1e3; }
  void SetSustain(double sustainMS) { _sustain = 1e-3 * sustainMS; }

 protected:
  virtual void MixImplementation(const ControlValue *values,
                                 const Timing &timing, bool primary) override {
    double timePassed = 0.001 * (timing.TimeInMS() - _previousTime[primary]);
    _previousTime[primary] = timing.TimeInMS();
    const unsigned targetValue = values[0].UInt();
    if (targetValue != _fadingValue[primary]) {
      if (targetValue > _fadingValue[primary]) {
        _sustainTimer[primary] = _sustain;
        if (_fadeUpSpeed == 0.0)
          _fadingValue[primary] = targetValue;
        else {
          unsigned stepSize = unsigned(std::min<double>(
              timePassed * _fadeUpSpeed * double(ControlValue::MaxUInt()),
              double(ControlValue::MaxUInt())));
          if (_fadingValue[primary] + stepSize > targetValue)
            _fadingValue[primary] = targetValue;
          else
            _fadingValue[primary] += stepSize;
        }
      } else {
        if (_sustainTimer[primary] > timePassed)
          _sustainTimer[primary] -= timePassed;
        else {
          timePassed -= _sustainTimer[primary];
          _sustainTimer[primary] = 0.0;
          if (_fadeDownSpeed == 0.0)
            _fadingValue[primary] = targetValue;
          else {
            unsigned stepSize = unsigned(std::min<double>(
                timePassed * _fadeDownSpeed * double(ControlValue::MaxUInt()),
                double(ControlValue::MaxUInt())));
            if (targetValue + stepSize > _fadingValue[primary])
              _fadingValue[primary] = targetValue;
            else
              _fadingValue[primary] -= stepSize;
          }
        }
      }
    }
    if (_fadingValue[primary] != 0) {
      setConnectedInputs(ControlValue(_fadingValue[primary]));
    }
  }

 private:
  unsigned _fadingValue[2];
  double _previousTime[2], _sustainTimer[2];

  double _fadeUpSpeed, _fadeDownSpeed;
  double _sustain;
};

}  // namespace glight::theatre

#endif
