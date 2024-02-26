#ifndef THEATRE_PRESETCOLLECTION_H_
#define THEATRE_PRESETCOLLECTION_H_

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
  PresetCollection() : _inputValue(0) {}
  PresetCollection(const std::string &name)
      : Controllable(name), _inputValue(0) {}
  ~PresetCollection() { Clear(); }

  void Clear() { _presetValues.clear(); }

  void SetFromCurrentSituation(Management &management);

  void SetFromCurrentFixtures(Management &management,
                              const std::set<Fixture *> &fixtures);

  size_t NInputs() const override { return 1; }

  ControlValue &InputValue(size_t) override { return _inputValue; }

  std::vector<Color> InputColors(size_t) const override;

  FunctionType InputType(size_t) const override { return FunctionType::Master; }

  size_t NOutputs() const override { return _presetValues.size(); }

  std::pair<const Controllable *, size_t> Output(size_t index) const override {
    return std::make_pair(&_presetValues[index]->GetControllable(),
                          _presetValues[index]->InputIndex());
  }

  void Mix(const Timing &timing, bool primary) override {
    unsigned leftHand = _inputValue.UInt();
    for (const std::unique_ptr<PresetValue> &pv : _presetValues) {
      unsigned rightHand = pv->Value().UInt();
      ControlValue value(
          ControlValue::Mix(leftHand, rightHand, MixStyle::Multiply));

      pv->GetControllable().MixInput(pv->InputIndex(), value);
    }
  }
  const std::vector<std::unique_ptr<PresetValue>> &PresetValues() const {
    return _presetValues;
  }
  PresetValue &AddPresetValue(const PresetValue &source) {
    _presetValues.emplace_back(new PresetValue(source));
    return *_presetValues.back();
  }
  PresetValue &AddPresetValue(Controllable &controllable, size_t input) {
    _presetValues.emplace_back(new PresetValue(controllable, input));
    return *_presetValues.back();
  }
  PresetValue &AddPresetValue(const PresetValue &source,
                              Controllable &controllable) {
    _presetValues.emplace_back(new PresetValue(source, controllable));
    return *_presetValues.back();
  }
  void RemovePresetValue(size_t index) {
    _presetValues.erase(_presetValues.begin() + index);
  }
  size_t Size() const { return _presetValues.size(); }

 private:
  ControlValue _inputValue;
  std::vector<std::unique_ptr<PresetValue>> _presetValues;
};

}  // namespace glight::theatre

#endif
