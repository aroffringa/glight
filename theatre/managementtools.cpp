#include "managementtools.h"

#include "color.h"
#include "fixture.h"
#include "fixturecontrol.h"
#include "management.h"

namespace glight::theatre {

void SetAllFixtures(
    Management& management,
    const std::vector<system::ObservingPtr<theatre::Fixture>>& fixtures,
    const Color& color) {
  for (const system::ObservingPtr<Fixture>& fixture : fixtures) {
    FixtureControl& control = *management.GetFixtureControl(*fixture);
    for (size_t i = 0; i != control.NInputs(); ++i) {
      const FunctionType type = control.InputType(i);
      if (IsColor(type)) {
        SourceValue* source = management.GetSourceValue(control, i);
        if (color == Color::Black())
          source->A().Set(0);
        else if (color == Color::White())
          source->A().Set(ControlValue::MaxUInt());
        else if (type == FunctionType::Red)
          source->A().Set(ControlValue::CharToValue(color.Red()));
        else if (type == FunctionType::Green)
          source->A().Set(ControlValue::CharToValue(color.Green()));
        else if (type == FunctionType::Blue)
          source->A().Set(ControlValue::CharToValue(color.Blue()));
      } else if (type == FunctionType::Master) {
        SourceValue* source = management.GetSourceValue(control, i);
        if (color == Color::Black())
          source->A().Set(0);
        else
          source->A().Set(ControlValue::MaxUInt());
      }
    }
  }
}

}  // namespace glight::theatre
