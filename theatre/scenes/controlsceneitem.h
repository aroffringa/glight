#ifndef THEATRE_CONTROLSCENEITEM_H_
#define THEATRE_CONTROLSCENEITEM_H_

#include "../controllable.h"
#include "../controlvalue.h"
#include "../timing.h"

#include "sceneitem.h"

namespace glight::theatre {

class ControlSceneItem final : public SceneItem {
 public:
  ControlSceneItem(Controllable &controllable, size_t input)
      : _controllable(controllable),
        _input(input),
        _startValue(ControlValue::Max()),
        _endValue(ControlValue::Max()) {}
  ~ControlSceneItem() {}

  ControlValue &StartValue() { return _startValue; }
  const ControlValue &StartValue() const { return _startValue; }

  ControlValue &EndValue() { return _endValue; }
  const ControlValue &EndValue() const { return _endValue; }

  std::string Description() const override { return _controllable.Name(); }
  void Mix(const Timing &timing, bool primary) override {
    const double ratio = (timing.TimeInMS() - OffsetInMS()) / DurationInMS();
    const ControlValue value(_startValue.UInt() * (1.0 - ratio) +
                             _endValue.UInt() * ratio);
    _controllable.MixInput(_input, value);
  }
  Controllable &GetControllable() const { return _controllable; }
  size_t GetInput() const { return _input; }

 private:
  Controllable &_controllable;
  size_t _input;
  ControlValue _startValue, _endValue;
};

}  // namespace glight::theatre

#endif
