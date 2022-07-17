#ifndef THEATRE_EFFECT_H_
#define THEATRE_EFFECT_H_

#include "folderobject.h"

#include "../theatre/controllable.h"

#include <sigc++/connection.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace glight::theatre {

class Effect : public Controllable {
 public:
  enum Type {
    AudioLevelType,
    ConstantValueType,
    CurveType,
    DelayType,
    FadeType,
    FlickerType,
    FluorescentStartType,
    InvertType,
    MusicActivationType,
    PulseType,
    RandomSelectType,
    ThresholdType
  };

  Effect(size_t nInputs) : _inputValues(nInputs, 0) {}

  virtual ~Effect() {
    while (!_connections.empty()) {
      RemoveConnection(_connections.size() - 1);
    }
  }

  virtual Type GetType() const = 0;

  static std::unique_ptr<Effect> Make(Type type);

  static std::string TypeToName(Type type);

  static Type NameToType(const std::string &name);

  static std::vector<Type> GetTypes();

  void AddConnection(Controllable &controllable, size_t input) {
    _connections.emplace_back(&controllable, input);
    _onDeleteConnections.emplace_back(
        controllable.SignalDelete().connect([&controllable, input, this]() {
          RemoveConnection(controllable, input);
        }));
  }

  void RemoveConnection(Controllable &controllable, size_t input) {
    std::vector<std::pair<Controllable *, size_t>>::iterator item =
        std::find(_connections.begin(), _connections.end(),
                  std::make_pair(&controllable, input));
    if (item == _connections.end())
      throw std::runtime_error(
          "RemoveConnection() called for unconnected controllable");
    // convert to index to also remove corresponding connection
    size_t index = item - _connections.begin();
    RemoveConnection(index);
  }

  void RemoveConnection(size_t index) {
    _connections.erase(_connections.begin() + index);
    _onDeleteConnections[index].disconnect();
    _onDeleteConnections.erase(_onDeleteConnections.begin() + index);
  }

  const std::vector<std::pair<Controllable *, size_t>> &Connections() const {
    return _connections;
  }

  std::unique_ptr<Effect> Copy() const;

  size_t NInputs() const final override { return _inputValues.size(); }

  ControlValue &InputValue(size_t index) final override {
    return _inputValues[index];
  }

  virtual FunctionType InputType(size_t) const override {
    return FunctionType::Master;
  }

  size_t NOutputs() const final override { return _connections.size(); }

  std::pair<Controllable *, size_t> Output(size_t index) const final override {
    return _connections[index];
  }

  void Mix(const class Timing &timing) final override {
    mix(_inputValues.data(), timing);
    for (ControlValue &v : _inputValues)  // TODO should be done in management
      v.Set(0);
  }

 protected:
  virtual void mix(const ControlValue *inputValues,
                   const class Timing &timing) = 0;

  void setConnectedInputs(const ControlValue &value) const {
    for (const std::pair<Controllable *, size_t> &connection : Connections())
      connection.first->MixInput(connection.second, value);
  }

 private:
  friend class EffectControl;

  std::vector<ControlValue> _inputValues;
  std::vector<std::pair<Controllable *, size_t>> _connections;
  std::vector<sigc::connection> _onDeleteConnections;
};

}  // namespace glight::theatre

#endif
