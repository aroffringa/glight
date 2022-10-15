#ifndef THEATRE_PRESETVALUE_H_
#define THEATRE_PRESETVALUE_H_

#include "controlvalue.h"

#include <sigc++/signal.h>

#include <string>

namespace glight::theatre {

class Controllable;

/**
 * A preset value links a controllable's input to a
 * value to which this input is set.
 * @author Andre Offringa
 */
class PresetValue {
 public:
  PresetValue(Controllable &controllable, size_t inputIndex)
      : _value(0), _controllable(&controllable), _inputIndex(inputIndex) {}

  /**
   * Copy the preset value. The delete signal is not copied.
   */
  PresetValue(const PresetValue &source)
      : _value(source._value),
        _controllable(source._controllable),
        _inputIndex(source._inputIndex) {}

  /**
   * Copy constructor that copies the source but associates it with the given
   * controllable.
   */
  PresetValue(const PresetValue &source, Controllable &controllable)
      : _value(source._value),
        _controllable(&controllable),
        _inputIndex(source._inputIndex) {}

  ~PresetValue() { _signalDelete(); }

  void SetValue(const ControlValue &value) { _value = value; }
  const ControlValue &Value() const { return _value; }
  ControlValue &Value() { return _value; }

  Controllable &GetControllable() const {
    return *_controllable;
  }

  size_t InputIndex() const { return _inputIndex; }

  bool IsIgnorable() const { return _value.UInt() == 0; }

  sigc::signal<void()> &SignalDelete() { return _signalDelete; }

  std::string Title() const;

  void Reconnect(Controllable &controllable, size_t inputIndex) {
    _controllable = &controllable;
    _inputIndex = inputIndex;
  }

 private:
  ControlValue _value;
  Controllable *_controllable;
  size_t _inputIndex;

  sigc::signal<void()> _signalDelete;
};

}  // namespace glight::theatre

#endif
