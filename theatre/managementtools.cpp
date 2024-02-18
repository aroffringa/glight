#include "managementtools.h"

#include "color.h"
#include "fixture.h"
#include "fixturecontrol.h"
#include "management.h"

namespace glight::theatre {

void SetAllFixtures(Management& management,
                    const std::vector<Fixture*> fixtures, const Color& color) {
  for (Fixture* fixture : fixtures) {
    FixtureControl& control = management.GetFixtureControl(*fixture);
    for (size_t i = 0; i != control.NInputs(); ++i) {
      if (IsColor(control.InputType(i))) {
        SourceValue* source = management.GetSourceValue(control, i);
        if (color == Color::Black())
          source->A().Set(0);
        else if (color == Color::White())
          source->A().Set(ControlValue::MaxUInt());
        else if (control.InputType(i) == FunctionType::Red)
          source->A().Set(ControlValue::CharToValue(color.Red()));
        else if (control.InputType(i) == FunctionType::Green)
          source->A().Set(ControlValue::CharToValue(color.Green()));
        else if (control.InputType(i) == FunctionType::Blue)
          source->A().Set(ControlValue::CharToValue(color.Blue()));
      }
    }
  }
}

}  // namespace glight::theatre
