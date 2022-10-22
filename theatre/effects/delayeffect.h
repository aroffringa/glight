#ifndef DELAY_EFFECT_H
#define DELAY_EFFECT_H

#include <array>
#include <vector>

#include "../effect.h"
#include "../timing.h"

namespace glight::theatre {

class DelayEffect final : public Effect {
 public:
  DelayEffect()
      : Effect(1),
        _previousTimestep{0, 0},
        _buffer{std::vector<std::pair<double, ControlValue>>(
                    2, std::pair<double, ControlValue>(0.0, 0)),
                std::vector<std::pair<double, ControlValue>>(
                    2, std::pair<double, ControlValue>(0.0, 0))},
        _bufferReadPos{0, 0},
        _bufferWritePos{0, 0},
        _delayInMS(100.0) {}

  virtual Effect::Type GetType() const override { return DelayType; }

  double DelayInMS() const { return _delayInMS; }
  void SetDelayInMS(double delayInMS) { _delayInMS = delayInMS; }

 protected:
  virtual void mix(const ControlValue *values, const Timing &timing,
                   bool primary) override {
    std::vector<std::pair<double, ControlValue>> &buffer = _buffer[primary];
    if (_previousTimestep[primary] == timing.TimestepNumber()) {
      unsigned prevWritePos =
          (_bufferWritePos[primary] + buffer.size() - 1) % buffer.size();
      buffer[prevWritePos].second.Set(
          ControlValue::Mix(buffer[prevWritePos].second.UInt(),
                            values[0].UInt(), MixStyle::Default));
    } else {
      _previousTimestep[primary] = timing.TimestepNumber();
      buffer[_bufferWritePos[primary]].first = timing.TimeInMS();
      buffer[_bufferWritePos[primary]].second = values[0];
      _bufferWritePos[primary] = (_bufferWritePos[primary] + 1) % buffer.size();
      if (_bufferWritePos[primary] == _bufferReadPos[primary]) {
        // Increase size of circular buffer
        size_t n = buffer.size();
        _bufferReadPos[primary] += n;
        buffer.resize(buffer.size() * 2);
        std::copy_backward(buffer.begin() + _bufferWritePos[primary],
                           buffer.begin() + n,
                           buffer.begin() + _bufferReadPos[primary]);
      }
      while (_bufferReadPos[primary] != _bufferWritePos[primary] &&
             buffer[_bufferReadPos[primary]].first + _delayInMS <
                 timing.TimeInMS()) {
        _bufferReadPos[primary] = (_bufferReadPos[primary] + 1) % buffer.size();
      }
    }
    for (const std::pair<Controllable *, size_t> &connection : Connections()) {
      connection.first->MixInput(connection.second,
                                 buffer[_bufferReadPos[primary]].second);
    }
  }

 private:
  unsigned _previousTimestep[2];
  std::array<std::vector<std::pair<double, ControlValue>>, 2> _buffer;
  size_t _bufferReadPos[2];
  size_t _bufferWritePos[2];

  double _delayInMS;
};

}  // namespace glight::theatre

#endif
