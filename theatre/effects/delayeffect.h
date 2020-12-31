#ifndef DELAY_EFFECT_H
#define DELAY_EFFECT_H

#include "../effect.h"
#include "../timing.h"

class DelayEffect : public Effect {
 public:
  DelayEffect()
      : Effect(1),
        _previousTimestep(0),
        _buffer(2, std::pair<double, ControlValue>(0.0, 0)),
        _bufferReadPos(0),
        _bufferWritePos(0),
        _delayInMS(100.0) {}

  virtual Effect::Type GetType() const override { return DelayType; }

  double DelayInMS() const { return _delayInMS; }
  void SetDelayInMS(double delayInMS) { _delayInMS = delayInMS; }

 protected:
  virtual void mix(const ControlValue *values,
                   const Timing &timing) final override {
    if (_previousTimestep == timing.TimestepNumber()) {
      unsigned prevWritePos =
          (_bufferWritePos + _buffer.size() - 1) % _buffer.size();
      _buffer[prevWritePos].second.Set(
          ControlValue::Mix(_buffer[prevWritePos].second.UInt(),
                            values[0].UInt(), ControlValue::Default));
    } else {
      _previousTimestep = timing.TimestepNumber();
      _buffer[_bufferWritePos].first = timing.TimeInMS();
      _buffer[_bufferWritePos].second = values[0];
      _bufferWritePos = (_bufferWritePos + 1) % _buffer.size();
      if (_bufferWritePos == _bufferReadPos) {
        // Increase size of circular buffer
        size_t n = _buffer.size();
        _bufferReadPos += n;
        _buffer.resize(_buffer.size() * 2);
        std::copy_backward(_buffer.begin() + _bufferWritePos,
                           _buffer.begin() + n,
                           _buffer.begin() + _bufferReadPos);
      }
      while (_bufferReadPos != _bufferWritePos &&
             _buffer[_bufferReadPos].first + _delayInMS < timing.TimeInMS()) {
        _bufferReadPos = (_bufferReadPos + 1) % _buffer.size();
      }
    }
    for (const std::pair<Controllable *, size_t> &connection : Connections()) {
      connection.first->MixInput(connection.second,
                                 _buffer[_bufferReadPos].second);
    }
  }

 private:
  unsigned _previousTimestep;
  std::vector<std::pair<double, ControlValue>> _buffer;
  size_t _bufferReadPos, _bufferWritePos;

  double _delayInMS;
};

#endif
