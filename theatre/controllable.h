#ifndef THEATRE_CONTROL_H_
#define THEATRE_CONTROL_H_

#include <array>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "color.h"
#include "controlvalue.h"
#include "fixturefunction.h"
#include "folderobject.h"
#include "input.h"

namespace glight::theatre {

class Controllable;
class Timing;

struct Connection {
  Controllable *to_controllable;
  size_t to_input_index;
  std::array<ControlValue, 2> values;
};

/**
 * A Controllable has a number of inputs and optionally some outputs
 * that this controllable controls.
 * @author Andre Offringa
 */
class Controllable : public FolderObject {
 public:
  Controllable() = default;

  Controllable(const Controllable &source) : FolderObject(source) {}

  Controllable(const std::string &name) : FolderObject(name) {}

  virtual size_t NInputs() const = 0;

  virtual ControlValue &InputValue(size_t index, bool primary) = 0;

  virtual FunctionType InputType(size_t index) const = 0;

  /**
   * Number of output connections this controllable has from one of its outputs
   * to the input of another controllable.
   */
  virtual size_t NConnections() const = 0;

  /**
   * Get information about a connection. A connection starts from the
   * output of this controllable and connects to the input of another
   * controllable.
   */
  virtual std::pair<const Controllable *, size_t> GetConnection(size_t index) const = 0;

  std::pair<Controllable *, size_t> GetConnection(size_t index) {
    const std::pair<const Controllable *, size_t> output =
        const_cast<const Controllable *>(this)->GetConnection(index);
    return std::make_pair(const_cast<Controllable *>(output.first), output.second);
  }

  virtual std::vector<Color> InputColors(size_t index) const {
    // Return the colours that it connects to
    std::vector<Color> colors;
    colors.reserve(NConnections());
    for (size_t o = 0; o != NConnections(); ++o) {
      const auto output = GetConnection(o);
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
  void MixInput(size_t index, ControlValue new_value, ControlValue previous_value, bool primary) {
    const FunctionType input_type = InputType(index);
    InputValue(index, primary) =
        theatre::MixInput(InputValue(index, primary), new_value, previous_value, input_type);
  }

  bool HasOutputConnection(const Controllable &controllable) const {
    for (size_t i = 0; i != NConnections(); ++i)
      if (GetConnection(i).first == &controllable) return true;
    return false;
  }

  /* Used for dependency analysis. */
  char VisitLevel() const { return _visitLevel; }

  void SetVisitLevel(char visitLevel) { _visitLevel = visitLevel; }

 protected:
 private:
  char _visitLevel = 0;
};

template <typename Condition>
void MixInputsIf(std::span<Input> inputs, ControlValue value,
                 std::vector<std::array<ControlValue, 2>> &connection_values, bool primary,
                 Condition condition) {
  for (size_t i = 0; i != inputs.size(); ++i) {
    if (condition(i)) {
      Input &input = inputs[i];
      input.GetControllable()->MixInput(input.InputIndex(), value, connection_values[i][primary],
                                        primary);
      connection_values[i][primary] = value;
    }
  }
}

}  // namespace glight::theatre

#endif
