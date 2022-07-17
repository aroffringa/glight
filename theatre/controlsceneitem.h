#ifndef THEATRE_CONTROLSCENEITEM_H_
#define THEATRE_CONTROLSCENEITEM_H_

#include "controllable.h"
#include "controlvalue.h"
#include "sceneitem.h"
#include "timing.h"

namespace glight::theatre {

/**
        @author Andre Offringa
*/
class ControlSceneItem : public SceneItem {
 public:
  ControlSceneItem(class Controllable &controllable, size_t input)
      : _controllable(controllable),
        _input(input),
        _startValue(ControlValue::Max()),
        _endValue(ControlValue::Max()) {}
  ~ControlSceneItem() {}
  ControlValue &StartValue() { return _startValue; }
  const ControlValue &StartValue() const { return _startValue; }

  ControlValue &EndValue() { return _endValue; }
  const ControlValue &EndValue() const { return _endValue; }

  virtual std::string Description() const { return _controllable.Name(); }
  virtual void Mix(unsigned *channelValues, unsigned universe,
                   const Timing &timing) {
    double ratio = (timing.TimeInMS() - OffsetInMS()) / DurationInMS();
    _controllable.MixInput(_input,
                           (unsigned int)(_startValue.UInt() * (1.0 - ratio) +
                                          _endValue.UInt() * ratio));
  }
  class Controllable &Controllable() const {
    return _controllable;
  }

 private:
  class Controllable &_controllable;
  size_t _input;
  ControlValue _startValue, _endValue;
};

}  // namespace glight::theatre

#endif
