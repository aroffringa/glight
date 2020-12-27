#include "../theatre/fixturecontrol.h"
#include "../theatre/management.h"
#include "../theatre/presetcollection.h"
#include "../theatre/theatre.h"
#include "../theatre/timing.h"

#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_SUITE(preset_collection)

BOOST_AUTO_TEST_CASE(Add) {
  Management management;
  FixtureType &fixtureType =
      management.Theatre().AddFixtureType(FixtureType::Light1Ch);
  Fixture &fixture = management.Theatre().AddFixture(fixtureType);
  FixtureControl &control = management.AddFixtureControl(fixture);
  PresetValue &value = management.AddPreset(control, 0);
  value.SetValue(ControlValue::Max());
  PresetCollection &presetCollection = management.AddPresetCollection();
  presetCollection.SetFromCurrentSituation(management);
  BOOST_CHECK_EQUAL(presetCollection.PresetValues().size(), 1);
  BOOST_CHECK_EQUAL(presetCollection.PresetValues()[0]->Value().UInt(),
                    ControlValue::MaxUInt());
}

BOOST_AUTO_TEST_CASE(SetValue) {
  Management management;
  FixtureType &fixtureType =
      management.Theatre().AddFixtureType(FixtureType::Light1Ch);
  Fixture &fixture = management.Theatre().AddFixture(fixtureType);
  fixture.SetChannel(100);
  FixtureControl &fixtureControl = management.AddFixtureControl(fixture);
  PresetValue &value = management.AddPreset(fixtureControl, 0);
  value.SetValue(ControlValue::Max());
  PresetCollection &presetCollection = management.AddPresetCollection();
  presetCollection.SetFromCurrentSituation(management);

  fixtureControl.InputValue(0) = ControlValue::Zero();
  presetCollection.InputValue(0) = ControlValue::Zero();
  presetCollection.MixInput(0, ControlValue::Max());

  std::vector<unsigned> values(512, 0);
  Timing timing(0.0, 0, 0, 0, 0);
  // Mix controls in order of dependencies
  presetCollection.Mix(values.data(), 0, timing);
  fixtureControl.Mix(values.data(), 0, timing);
  for (size_t i = 0; i != 512; ++i) {
    if (i == 100) {
      // it's not accurately Max, because of truncations.
      BOOST_CHECK_GE(values[100], (ControlValue::MaxUInt() >> 8) * 255);
    } else
      BOOST_CHECK_EQUAL(values[i], 0);
  }
}

BOOST_AUTO_TEST_SUITE_END()
