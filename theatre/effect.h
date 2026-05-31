#ifndef THEATRE_EFFECT_H_
#define THEATRE_EFFECT_H_

#include "controllable.h"
#include "effecttype.h"
#include "folderobject.h"

#include <sigc++/connection.h>

#include <algorithm>
#include <array>
#include <stdexcept>
#include <vector>

namespace glight::theatre {

class Effect : public Controllable {
 public:
  Effect(size_t n_inputs) : input_values_(n_inputs, ControlValue()) {}

  virtual ~Effect() {
    while (!connections_.empty()) {
      RemoveConnection(connections_.size() - 1);
    }
  }

  virtual EffectType GetType() const = 0;

  static std::unique_ptr<Effect> Make(EffectType type);

  void AddConnection(Controllable &controllable, size_t input) {
    connections_.emplace_back(&controllable, input);
    connection_values_.push_back({ControlValue::Zero(), ControlValue::Zero()});
    on_delete_connections_.emplace_back(controllable.SignalDelete().connect(
        [&controllable, input, this]() { RemoveConnection(controllable, input); }));
  }

  void RemoveConnection(Controllable &controllable, size_t input) {
    std::vector<std::pair<Controllable *, size_t>>::iterator item =
        std::find(connections_.begin(), connections_.end(), std::make_pair(&controllable, input));
    if (item == connections_.end())
      throw std::runtime_error("RemoveConnection() called for unconnected controllable");
    // convert to index to also remove corresponding connection
    size_t index = item - connections_.begin();
    RemoveConnection(index);
  }

  void RemoveConnection(size_t index) {
    connections_.erase(connections_.begin() + index);
    connection_values_.erase(connection_values_.begin() + index);
    on_delete_connections_[index].disconnect();
    on_delete_connections_.erase(on_delete_connections_.begin() + index);
  }

  const std::vector<std::pair<Controllable *, size_t>> &Connections() const { return connections_; }

  std::unique_ptr<Effect> Copy() const;

  size_t NInputs() const final override { return input_values_.size(); }

  ControlValue &InputValue(size_t index) final override { return input_values_[index]; }

  virtual FunctionType InputType(size_t) const override { return FunctionType::Master; }

  size_t NConnections() const final override { return connections_.size(); }

  std::pair<const Controllable *, size_t> GetConnection(size_t index) const final override {
    return connections_[index];
  }

  void Mix(const Timing &timing, bool primary) final override {
    MixImplementation(input_values_.data(), timing, primary);
  }

 protected:
  virtual void MixImplementation(const ControlValue *inputValues, const Timing &timing,
                                 bool primary) = 0;

  /**
   * Output the provided value to all output connections. Because
   * inputs are where the values are stored, this implies that this
   * function sets the inputs of the connected objects.
   */
  void setAllOutputs(ControlValue value, bool primary) const {
    for (size_t i = 0; i != connections_.size(); ++i) {
      MixConnection(i, value, primary);
    }
  }

  void MixConnection(size_t connection_index, ControlValue value, bool primary) const {
    const std::pair<Controllable *, size_t> &connection = connections_[connection_index];
    connection.first->MixInput(connection.second, value,
                               connection_values_[connection_index][primary]);
  }

 private:
  friend class EffectControl;

  std::vector<ControlValue> input_values_;
  std::vector<std::pair<Controllable *, size_t>> connections_;
  std::vector<std::array<ControlValue, 2>> connection_values_;
  std::vector<sigc::connection> on_delete_connections_;
};

}  // namespace glight::theatre

#endif
