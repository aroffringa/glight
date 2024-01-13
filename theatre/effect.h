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
  Effect(size_t n_inputs) : input_values_(n_inputs, ControlValue()) {}

  virtual ~Effect() {
    while (!outputs_.empty()) {
      RemoveConnection(outputs_.size() - 1);
    }
  }

  virtual EffectType GetType() const = 0;

  static std::unique_ptr<Effect> Make(EffectType type);

  void AddConnection(Controllable &controllable, size_t input) {
    outputs_.emplace_back(&controllable, input);
    on_delete_connections_.emplace_back(
        controllable.SignalDelete().connect([&controllable, input, this]() {
          RemoveConnection(controllable, input);
        }));
  }

  void RemoveConnection(Controllable &controllable, size_t input) {
    std::vector<std::pair<Controllable *, size_t>>::iterator item = std::find(
        outputs_.begin(), outputs_.end(), std::make_pair(&controllable, input));
    if (item == outputs_.end())
      throw std::runtime_error(
          "RemoveConnection() called for unconnected controllable");
    // convert to index to also remove corresponding connection
    size_t index = item - outputs_.begin();
    RemoveConnection(index);
  }

  void RemoveConnection(size_t index) {
    outputs_.erase(outputs_.begin() + index);
    on_delete_connections_[index].disconnect();
    on_delete_connections_.erase(on_delete_connections_.begin() + index);
  }

  const std::vector<std::pair<Controllable *, size_t>> &Connections() const {
    return outputs_;
  }

  std::unique_ptr<Effect> Copy() const;

  size_t NInputs() const final override { return input_values_.size(); }

  ControlValue &InputValue(size_t index) final override {
    return input_values_[index];
  }

  virtual FunctionType InputType(size_t) const override {
    return FunctionType::Master;
  }

  size_t NOutputs() const final override { return outputs_.size(); }

  std::pair<const Controllable *, size_t> Output(
      size_t index) const final override {
    return outputs_[index];
  }

  void Mix(const Timing &timing, bool primary) final override {
    MixImplementation(input_values_.data(), timing, primary);
  }

 protected:
  virtual void MixImplementation(const ControlValue *inputValues,
                                 const Timing &timing, bool primary) = 0;

  /**
   * Output the provided value to all output connections. Because
   * inputs are where the values are stored, this implies that this
   * function sets the inputs of the connected objects.
   */
  void setAllOutputs(const ControlValue &value) const {
    for (const std::pair<Controllable *, size_t> &connection : Connections())
      connection.first->MixInput(connection.second, value);
  }

 private:
  friend class EffectControl;

  std::vector<ControlValue> input_values_;
  std::vector<std::pair<Controllable *, size_t>> outputs_;
  std::vector<sigc::connection> on_delete_connections_;
};

}  // namespace glight::theatre

#endif
