#include "presetcollection.h"

#include "fixturecontrol.h"

#include <map>

namespace glight::theatre {

struct FixtureColorSum {
  unsigned red = 0;
  unsigned green = 0;
  unsigned blue = 0;
  size_t n = 0;
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
      const Color color =
          GetFunctionColor(fixture_control->InputType(preset->InputIndex()));
      FixtureColorSum& sum = fixture_sum[fixture_control];
      sum.red +=
          color.Red() * value.UInt() / glight::theatre::ControlValue::MaxUInt();
      sum.green += color.Green() * value.UInt() /
                   glight::theatre::ControlValue::MaxUInt();
      sum.blue += color.Blue() * value.UInt() /
                  glight::theatre::ControlValue::MaxUInt();
      sum.n = fixture_control->GetFixture().Type().ColorScalingValue();
    } else {
      const std::vector<Color> colors =
          controllable.InputColors(preset->InputIndex());
      result.insert(result.end(), colors.begin(), colors.end());
    }
  }
  for (const std::pair<const FixtureControl* const, FixtureColorSum>& p :
       fixture_sum) {
    const FixtureColorSum& sum = p.second;
    result.emplace_back(sum.red * 255 / sum.n, sum.green * 255 / sum.n,
                        sum.blue * 255 / sum.n);
  }
  return result;
}

}  // namespace glight::theatre
