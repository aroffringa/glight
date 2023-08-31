#ifndef RANDOM_EFFECT_H
#define RANDOM_EFFECT_H

#include "../effect.h"
#include "../timing.h"

#include <vector>

namespace glight::theatre {

class RandomSelectEffect final : public Effect {
 public:
  RandomSelectEffect()
      : Effect(1),
        _active{false, false},
        _startTime{0.0, 0.0},
        _delay(10000.0),
        _count(1){};

  virtual EffectType GetType() const override {
    return EffectType::RandomSelect;
  }

  double Delay() const { return _delay; }
  void SetDelay(double delay) { _delay = delay; }

  size_t Count() const { return _count; }
  void SetCount(size_t count) { _count = count; }

 private:
  virtual void MixImplementation(const ControlValue *values,
                                 const Timing &timing,
                                 bool primary) final override {
    size_t count = std::min(_count, Connections().size());
    if (values[0] && count != 0) {
      std::vector<size_t> &activeConnections = _activeConnections[primary];
      if (Connections().size() != activeConnections.size()) {
        activeConnections.resize(Connections().size());
        for (size_t i = 0; i != activeConnections.size(); ++i)
          activeConnections[i] = i;
        std::shuffle(activeConnections.begin(), activeConnections.end(),
                     timing.RNG());
      }
      if (!_active[primary] ||
          timing.TimeInMS() - _startTime[primary] > _delay) {
        _active[primary] = true;
        _startTime[primary] = timing.TimeInMS();
        std::shuffle(activeConnections.begin(), activeConnections.end(),
                     timing.RNG());
      }
      for (size_t i = 0; i != count; ++i) {
        if (activeConnections[i] < Connections().size()) {
          const std::pair<Controllable *, size_t> &connection =
              Connections()[activeConnections[i]];
          connection.first->MixInput(connection.second, values[0]);
        }
      }
    } else {
      _active[primary] = false;
    }
  }

  bool _active[2];
  std::array<std::vector<size_t>, 2> _activeConnections;
  double _startTime[2];

  double _delay;
  size_t _count;
};

}  // namespace glight::theatre

#endif
