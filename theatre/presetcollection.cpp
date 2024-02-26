#include "presetcollection.h"

#include "fixturecontrol.h"
#include "management.h"

#include <map>

namespace glight::theatre {

struct FixtureColorSum {
  unsigned red = 0;
  unsigned green = 0;
  unsigned blue = 0;
};

std::vector<Color> PresetCollection::InputColors(size_t /*index*/) const {
  // Calculate average color over inputs when from the same preset collection
  std::vector<Color> result;
  std::map<const FixtureControl*, FixtureColorSum> fixture_sum;
  for (const std::unique_ptr<PresetValue>& preset : _presetValues) {
    ControlValue value = preset->Value();
    const Controllable& controllable = preset->GetControllable();
    const FixtureControl* fixture_control =
        dynamic_cast<const FixtureControl*>(&controllable);
    if (fixture_control) {
      const FunctionType input_type =
          fixture_control->InputType(preset->InputIndex());
      if (IsColor(input_type)) {
        const Color color = GetFunctionColor(input_type);
        FixtureColorSum& sum = fixture_sum[fixture_control];
        constexpr unsigned m = glight::theatre::ControlValue::MaxUInt();
        sum.red += color.Red() * std::min(m, value.UInt()) / m;
        sum.green += color.Green() * std::min(m, value.UInt()) / m;
        sum.blue += color.Blue() * std::min(m, value.UInt()) / m;
        // Ignore the color scaling, because it leads to darker colors
        // sum.n = fixture_control->GetFixture().Type().ColorScalingValue();
      }
    } else {
      const std::vector<Color> colors =
          controllable.InputColors(preset->InputIndex());
      result.insert(result.end(), colors.begin(), colors.end());
    }
  }
  for (const std::pair<const FixtureControl* const, FixtureColorSum>& p :
       fixture_sum) {
    const FixtureColorSum& sum = p.second;
    result.emplace_back(std::min(255u, sum.red), std::min(255u, sum.green),
                        std::min(255u, sum.blue));
  }
  return result;
}

void PresetCollection::SetFromCurrentSituation(Management& management) {
  Clear();
  std::vector<std::unique_ptr<SourceValue>>& values = management.SourceValues();
  for (std::unique_ptr<SourceValue>& sv : values) {
    if (!sv->A().IsIgnorable() && (&sv->GetControllable()) != this) {
      std::unique_ptr<PresetValue>& value =
          _presetValues.emplace_back(std::make_unique<PresetValue>(
              sv->GetControllable(), sv->InputIndex()));
      value->SetValue(sv->A().Value());
    }
  }
}

void PresetCollection::SetFromCurrentFixtures(
    Management& management, const std::set<Fixture*>& fixtures) {
  Clear();
  std::vector<std::unique_ptr<SourceValue>>& values = management.SourceValues();
  for (std::unique_ptr<SourceValue>& sv : values) {
    if (!sv->A().IsIgnorable() && (&sv->GetControllable()) != this) {
      FixtureControl* control =
          dynamic_cast<FixtureControl*>(&sv->GetControllable());
      if (control) {
        if (fixtures.contains(&control->GetFixture())) {
          std::unique_ptr<PresetValue>& value =
              _presetValues.emplace_back(std::make_unique<PresetValue>(
                  sv->GetControllable(), sv->InputIndex()));
          value->SetValue(sv->A().Value());
        }
      }
    }
  }
}

}  // namespace glight::theatre
