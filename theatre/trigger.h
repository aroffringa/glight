#ifndef THEATRE_TRIGGER_H_
#define THEATRE_TRIGGER_H_

#include <string>

namespace glight::theatre {

enum class TriggerType { Delay, Sync, Beat };

inline std::string ToString(TriggerType trigger_style) {
  switch (trigger_style) {
    default:
    case TriggerType::Delay:
      return "delay";
    case TriggerType::Sync:
      return "sync";
    case TriggerType::Beat:
      return "beat";
  }
}

inline TriggerType GetTriggerType(const std::string& str) {
  if (str == "sync")
    return TriggerType::Sync;
  else if (str == "beat")
    return TriggerType::Beat;
  else
    return TriggerType::Delay;
}

/**
 * @author Andre Offringa
 */
class Trigger {
 public:
  Trigger()
      : _type(TriggerType::Delay),
        _delayInMs(500.0),
        _delaySynced(1),
        _delayInBeats(1.0) {}

  double DelayInMs() const { return _delayInMs; }
  void SetDelayInMs(double delay) { _delayInMs = delay; }

  unsigned DelayInSyncs() const { return _delaySynced; }
  void SetDelayInSyncs(unsigned syncs) { _delaySynced = syncs; }

  double DelayInBeats() const { return _delayInBeats; }
  void SetDelayInBeats(double delay) { _delayInBeats = delay; }

  TriggerType Type() const { return _type; }
  void SetType(TriggerType type) { _type = type; }

  std::string ToString() const {
    switch (_type) {
      case TriggerType::Delay:
        return std::to_string(_delayInMs / 1000.0) + " s";
      case TriggerType::Sync:
        return std::to_string(_delaySynced) + " syncs";
      case TriggerType::Beat:
        return std::to_string(_delayInBeats) + " beats";
    }
    return std::string();
  }

 private:
  TriggerType _type;
  double _delayInMs;
  unsigned _delaySynced;
  double _delayInBeats;
};

}  // namespace glight::theatre

#endif
