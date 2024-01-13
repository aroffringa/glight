#ifndef THEATRE_CONTROL_H_
#define THEATRE_CONTROL_H_

#include <string>

#include "color.h"
#include "controlvalue.h"
#include "fixturefunction.h"
#include "folderobject.h"

namespace glight::theatre {

class Timing;

/**
 * A Controllable has a number of inputs and optionally some outputs
 * that this controllable controls.
 * @author Andre Offringa
 */
class Controllable : public FolderObject {
 public:
  Controllable() : _visitLevel(0) {}

  Controllable(const Controllable &source)
      : FolderObject(source), _visitLevel(0) {}

  Controllable(const std::string &name) : FolderObject(name), _visitLevel(0) {}

  virtual size_t NInputs() const = 0;

  virtual ControlValue &InputValue(size_t index) = 0;

  virtual FunctionType InputType(size_t index) const = 0;

  virtual size_t NOutputs() const = 0;

  virtual std::pair<const Controllable *, size_t> Output(
      size_t index) const = 0;

  std::pair<Controllable *, size_t> Output(size_t index) {
    const std::pair<const Controllable *, size_t> output =
        const_cast<const Controllable *>(this)->Output(index);
    return std::make_pair(const_cast<Controllable *>(output.first),
                          output.second);
  }

  virtual std::vector<Color> InputColors([[maybe_unused]] size_t index) const {
    // Return the colours that it connects to
    std::vector<Color> colors;
    colors.reserve(NOutputs());
    for (size_t o = 0; o != NOutputs(); ++o) {
      const auto output = Output(o);
      const std::vector<Color> c = output.first->InputColors(output.second);
      colors.insert(colors.end(), c.begin(), c.end());
    }
    return colors;
  }

  /**
   * Combine all inputs and outputs controlled by this controllable.
   * Before this function is called, all input values that this
   * controllable depends on have been set.
   */
  virtual void Mix(const Timing &timing, bool primary) = 0;

  std::string InputName(size_t index) const {
    if (NInputs() == 1)
      return Name();
    else
      return Name() + " (" + AbbreviatedFunctionType(InputType(index)) + ")";
  }

  /**
   * Sets the value at the controllable's input.
   */
  void MixInput(size_t index, const ControlValue &value) {
    const unsigned mixVal = ControlValue::Mix(InputValue(index).UInt(),
                                              value.UInt(), MixStyle::Default);
    InputValue(index) = ControlValue(mixVal);
  }

  bool HasOutputConnection(const Controllable &controllable) const {
    for (size_t i = 0; i != NOutputs(); ++i)
      if (Output(i).first == &controllable) return true;
    return false;
  }

  /* Used for dependency analysis. */
  char VisitLevel() const { return _visitLevel; }

  void SetVisitLevel(char visitLevel) { _visitLevel = visitLevel; }

 private:
  ControlValue _inputValue;
  char _visitLevel;
};

}  // namespace glight::theatre

#endif
