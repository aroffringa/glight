#ifndef THEATRE_FIXTURE_MODE_H_
#define THEATRE_FIXTURE_MODE_H_

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "color.h"
#include "fixturemodefunction.h"
#include "namedobject.h"
#include "stockfixture.h"
#include "valuesnapshot.h"

namespace glight::theatre {

class Fixture;
class FixtureType;

/**
 *  @author Andre Offringa
 */
class FixtureMode : public NamedObject {
 public:
  FixtureMode(FixtureType& parent_type) : type_(&parent_type) {}

  FixtureMode(const FixtureMode &fixture_mode) = default;

  FixtureType& Type() { return *type_; }
  const FixtureType& Type() const { return *type_; }

  Color GetColor(const Fixture &fixture, const ValueSnapshot &snapshot,
                 size_t shape_index) const;

  /**
   * Determine the rotation speed of the fixture corresponding with the
   * snapshot. 0 is no rotation, +/- 2^24 is 100 times per second (the max).
   * Positive is clockwise rotation.
   */
  int GetRotationSpeed(const Fixture &fixture, const ValueSnapshot &snapshot,
                       size_t shape_index) const;

  double GetPan(const Fixture &fixture, const ValueSnapshot &snapshot,
                size_t shape_index) const;

  double GetTilt(const Fixture &fixture, const ValueSnapshot &snapshot,
                 size_t shape_index) const;

  double GetZoom(const Fixture &fixture, const ValueSnapshot &snapshot,
                 size_t shape_index) const;

  double GetPower(const Fixture &fixture, const ValueSnapshot &snapshot) const;

  const std::vector<FixtureModeFunction> &Functions() const {
    return functions_;
  }

  void SetFunctions(const std::vector<FixtureModeFunction> &functions) {
    functions_ = functions;
    UpdateFunctions();
  }
  void SetFunctions(std::vector<FixtureModeFunction> &&functions) {
    functions_ = std::move(functions);
    UpdateFunctions();
  }
  unsigned ColorScalingValue() const { return scaling_value_; }

 private:
  void UpdateFunctions();

  std::vector<FixtureModeFunction> functions_;
  FixtureType* type_;
  unsigned scaling_value_;
};

inline std::string FunctionSummary(const FixtureMode &fixture_mode) {
  const std::vector<FixtureModeFunction> &functions = fixture_mode.Functions();
  if (functions.empty())
    return "-";
  else {
    std::ostringstream s;
    s << AbbreviatedFunctionType(functions.front().Type());
    for (size_t i = 1; i != functions.size(); ++i) {
      s << "-" << AbbreviatedFunctionType(functions[i].Type());
    }
    return s.str();
  }
}

}  // namespace glight::theatre

#endif
