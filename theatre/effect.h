#ifndef THEATRE_EFFECT_H_
#define THEATRE_EFFECT_H_

#include "effecttype.h"
#include "folderobject.h"

#include "../theatre/controllable.h"

#include <sigc++/connection.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace glight::theatre {

class Effect : public Controllable {
 public:
  Effect(size_t nInputs) : _inputValues(nInputs, ControlValue()) {}

  virtual ~Effect() {
    while (!_connections.empty()) {
      RemoveConnection(_connections.size() - 1);
    }
  }

  virtual EffectType GetType() const = 0;

  static std::unique_ptr<Effect> Make(EffectType type);

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

  size_t NInputs() const override { return _inputValues.size(); }

  ControlValue &InputValue(size_t index) final override {
    return _inputValues[index];
  }

  virtual FunctionType InputType(size_t) const override {
    return FunctionType::Master;
  }

  size_t NOutputs() const final override { return _connections.size(); }

  std::pair<const Controllable *, size_t> Output(
      size_t index) const final override {
    return _connections[index];
  }

  void Mix(const Timing &timing, bool primary) final override {
    mix(_inputValues.data(), timing, primary);
  }

 protected:
  virtual void mix(const ControlValue *inputValues, const Timing &timing,
                   bool primary) = 0;

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
