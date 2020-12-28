#ifndef VALUESNAPSHOT_H
#define VALUESNAPSHOT_H

#include "dmxchannel.h"
#include "valueuniversesnapshot.h"

class ValueSnapshot {
public:
  ValueSnapshot() {}

  ValueSnapshot(size_t universeCount) { SetUniverseCount(universeCount); }

  ValueSnapshot(const ValueSnapshot &source) {
    for (const std::unique_ptr<ValueUniverseSnapshot> &snapshot :
         source._universeValues)
      _universeValues.emplace_back(
          std::make_unique<ValueUniverseSnapshot>(*snapshot));
  }

  ValueSnapshot(ValueSnapshot &&source) = default;

  ValueSnapshot &operator=(const ValueSnapshot &rhs) {
    _universeValues.clear();
    _universeValues.reserve(rhs._universeValues.size());
    for (const std::unique_ptr<ValueUniverseSnapshot> &snapshot :
         rhs._universeValues)
      _universeValues.emplace_back(new ValueUniverseSnapshot(*snapshot));
    return *this;
  }

  ValueSnapshot &operator=(ValueSnapshot &&rhs) = default;

  ~ValueSnapshot() {}

  void Clear() { _universeValues.clear(); }

  void SetUniverseCount(size_t count) {
    while (count < _universeValues.size()) {
      _universeValues.erase(--_universeValues.end());
    }
    while (count > _universeValues.size()) {
      _universeValues.emplace_back(std::make_unique<ValueUniverseSnapshot>());
    }
  }

  unsigned char GetValue(const DmxChannel &channel) const {
    return _universeValues[channel.Universe()]->GetValue(channel.Channel());
  }

  ValueUniverseSnapshot &GetUniverseSnapshot(size_t index) const {
    return *_universeValues[index];
  }

private:
  std::vector<std::unique_ptr<ValueUniverseSnapshot>> _universeValues;
};

#endif // VALUESNAPSHOT_H
