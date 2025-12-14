#ifndef RANDOM_EFFECT_H
#define RANDOM_EFFECT_H

#include "../effect.h"
#include "../timing.h"

#include <vector>

namespace glight::theatre {

class RandomSelectEffect final : public Effect {
 public:
  RandomSelectEffect() : Effect(1){};

  virtual EffectType GetType() const final { return EffectType::RandomSelect; }

  double Delay() const { return _delay; }
  void SetDelay(double delay) { _delay = delay; }

  size_t Count() const { return _count; }
  void SetCount(size_t count) { _count = count; }

 private:
  virtual void MixImplementation(const ControlValue *values,
                                 const Timing &timing, bool primary) final {
    size_t n_active = std::min(_count, Connections().size());
    if (values[0] && n_active != 0) {
      std::vector<size_t> &activeConnections = _activeConnections[primary];
      if (Connections().size() != activeConnections.size()) {
        activeConnections.resize(Connections().size());
        for (size_t i = 0; i != activeConnections.size(); ++i)
          activeConnections[i] = i;
        Shuffle(activeConnections, timing, false);
      }
      if (!_active[primary] ||
          timing.TimeInMS() - _startTime[primary] > _delay) {
        _active[primary] = true;
        _startTime[primary] = timing.TimeInMS();
        Shuffle(activeConnections, timing, true);
      }
      for (size_t i = 0; i != n_active; ++i) {
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

  void Shuffle(std::vector<size_t> &list, const Timing &timing,
               bool is_restart) {
    if (!list.empty()) {
      // If only one output is active, require the output to change
      // when reshuffling if it is a restart.
      bool must_change = _count == 1 && list.size() > 1 && is_restart;
      size_t first_active = list.front();
      do {
        std::shuffle(list.begin(), list.end(), timing.RNG());
      } while (must_change && first_active == list.front());
    }
  }

  bool _active[2] = {false, false};
  std::array<std::vector<size_t>, 2> _activeConnections;
  double _startTime[2] = {0.0, 0.0};

  double _delay = 10000.0;
  size_t _count = 1;
};

}  // namespace glight::theatre

#endif
