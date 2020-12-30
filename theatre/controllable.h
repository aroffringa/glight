#ifndef CONTROL_H
#define CONTROL_H

#include <string>

#include "color.h"
#include "controlvalue.h"
#include "fixturefunction.h"
#include "folderobject.h"

/**
        @author Andre Offringa
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

  virtual Color InputColor(size_t index) const {
    if (NOutputs() == 0)
      return Color::Black();
    else {
      // Return the average colour that it connects to
      unsigned r = 0, g = 0, b = 0;
      for (size_t o = 0; o != NOutputs(); ++o) {
        auto output = Output(o);
        Color c = output.first->InputColor(output.second);
        r += c.Red();
        g += c.Green();
        b += c.Blue();
      }
      return Color(r / NOutputs(), g / NOutputs(), b / NOutputs());
    }
  }

  std::string InputName(size_t index) const {
    if (NInputs() == 1)
      return Name();
    else
      return Name() + " (" + AbbreviatedFunctionType(InputType(index)) + ")";
  }

  void MixInput(size_t index, const ControlValue &value) {
    unsigned mixVal = ControlValue::Mix(InputValue(index).UInt(), value.UInt(),
                                        ControlValue::Default);
    InputValue(index) = ControlValue(mixVal);
  }

  virtual size_t NOutputs() const = 0;

  virtual std::pair<Controllable *, size_t> Output(size_t index) const = 0;

  virtual void Mix(unsigned *channelValues, unsigned universe,
                   const class Timing &timing) = 0;

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

#endif
