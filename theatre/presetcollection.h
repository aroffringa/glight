#ifndef THEATRE_PRESETCOLLECTION_H_
#define THEATRE_PRESETCOLLECTION_H_

#include <memory>
#include <vector>

#include "controllable.h"
#include "dmxchannel.h"
#include "presetvalue.h"
#include "sourcevalue.h"

namespace glight::theatre {
  
class Management;

/**
 * @author Andre Offringa
 */
class PresetCollection : public Controllable {
 public:
  PresetCollection() : _inputValue(0) {}
  PresetCollection(const std::string &name)
      : Controllable(name), _inputValue(0) {}
  ~PresetCollection() { Clear(); }

  void Clear() { _presetValues.clear(); }
  inline void SetFromCurrentSituation(const Management &management);

  size_t NInputs() const final override { return 1; }

  ControlValue &InputValue(size_t) final override { return _inputValue; }

  virtual FunctionType InputType(size_t) const final override {
    return FunctionType::Master;
  }

  size_t NOutputs() const final override { return _presetValues.size(); }

  std::pair<Controllable *, size_t> Output(size_t index) const final override {
    return std::make_pair(&_presetValues[index]->Controllable(),
                          _presetValues[index]->InputIndex());
  }

  void Mix(const Timing &timing) final override {
    unsigned leftHand = _inputValue.UInt();
    for (const std::unique_ptr<PresetValue> &pv : _presetValues) {
      unsigned rightHand = pv->Value().UInt();
      ControlValue value(
          ControlValue::Mix(leftHand, rightHand, MixStyle::Multiply));

      pv->Controllable().MixInput(pv->InputIndex(), value);
    }
  }
  const std::vector<std::unique_ptr<PresetValue>> &PresetValues() const {
    return _presetValues;
  }
  PresetValue &AddPresetValue(const class PresetValue &source) {
    _presetValues.emplace_back(new PresetValue(source));
    return *_presetValues.back();
  }
  PresetValue &AddPresetValue(class Controllable &controllable, size_t input) {
    _presetValues.emplace_back(new PresetValue(controllable, input));
    return *_presetValues.back();
  }
  PresetValue &AddPresetValue(const class PresetValue &source,
                              class Controllable &controllable) {
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

#include "management.h"

namespace glight::theatre {

void PresetCollection::SetFromCurrentSituation(const Management &management) {
  Clear();
  const std::vector<std::unique_ptr<SourceValue>> &values =
      management.SourceValues();
  for (const std::unique_ptr<SourceValue> &sv : values) {
    if (!sv->IsIgnorable() && (&sv->Controllable()) != this)
      _presetValues.emplace_back(new PresetValue(sv->Preset()));
  }
}

}  // namespace glight::theatre

#endif
