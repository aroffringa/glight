#ifndef THEATRE_PRESETCOLLECTION_H_
#define THEATRE_PRESETCOLLECTION_H_

#include <array>
#include <memory>
#include <set>
#include <vector>

#include "controllable.h"
#include "dmxchannel.h"
#include "presetvalue.h"
#include "sourcevalue.h"

namespace glight::theatre {

class Fixture;
class Management;

/**
 * @author Andre Offringa
 */
class PresetCollection final : public Controllable {
 public:
  PresetCollection() {}
  PresetCollection(const std::string &name) : Controllable(name) {}
  ~PresetCollection() { Clear(); }

  void Clear() {
    _presetValues.clear();
    connection_values_.clear();
  }

  void SetFromCurrentSituation(Management &management);

  void SetFromCurrentFixtures(Management &management,
                              const std::set<system::ObservingPtr<Fixture>, std::less<>> &fixtures);

  size_t NInputs() const override { return 1; }

  ControlValue &InputValue(size_t, bool primary) override { return _inputValue[primary]; }

  std::vector<Color> InputColors(size_t) const override;

  FunctionType InputType(size_t) const override { return FunctionType::Master; }

  size_t NConnections() const override { return _presetValues.size(); }

  std::pair<const Controllable *, size_t> GetConnection(size_t index) const override {
    return std::make_pair(&_presetValues[index]->GetControllable(),
                          _presetValues[index]->InputIndex());
  }

  void Mix(const Timing &timing, bool primary) override {
    for (size_t i = 0; i != _presetValues.size(); ++i) {
      const std::unique_ptr<PresetValue> &pv = _presetValues[i];
      const ControlValue value = _inputValue[primary] * pv->Value();

      pv->GetControllable().MixInput(pv->InputIndex(), value, connection_values_[i][primary],
                                     primary);
      connection_values_[i][primary] = value;
    }
  }
  const std::vector<std::unique_ptr<PresetValue>> &PresetValues() const { return _presetValues; }
  PresetValue &AddPresetValue(const PresetValue &source) {
    connection_values_.emplace_back();
    return *_presetValues.emplace_back(new PresetValue(source));
  }
  PresetValue &AddPresetValue(Controllable &controllable, size_t input) {
    connection_values_.emplace_back();
    return *_presetValues.emplace_back(new PresetValue(controllable, input));
  }
  PresetValue &AddPresetValue(const PresetValue &source, Controllable &controllable) {
    connection_values_.emplace_back();
    return *_presetValues.emplace_back(new PresetValue(source, controllable));
  }
  void RemovePresetValue(size_t index) {
    connection_values_.erase(connection_values_.begin() + index);
    _presetValues.erase(_presetValues.begin() + index);
  }
  size_t Size() const { return _presetValues.size(); }

 private:
  ControlValue _inputValue[2];
  std::vector<std::unique_ptr<PresetValue>> _presetValues;
  std::vector<std::array<ControlValue, 2>> connection_values_;
};

}  // namespace glight::theatre

#endif
