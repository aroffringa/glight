#include "system/settings.h"

#include "theatre/fixturecontrol.h"
#include "theatre/fixturetype.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"
#include "theatre/theatre.h"
#include "theatre/timing.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;
using glight::system::TrackablePtr;

BOOST_AUTO_TEST_SUITE(preset_collection)

BOOST_AUTO_TEST_CASE(Add) {
  const glight::system::Settings settings;
  Management management(settings);
  FixtureType &fixtureType =
      *management.GetTheatre().AddFixtureType(StockFixture::Light1Ch);
  Fixture &fixture =
      *management.GetTheatre().AddFixture(fixtureType.Modes().front());
  Controllable *control = management.AddFixtureControl(fixture).Get();
  SourceValue &value = management.AddSourceValue(*control, 0);
  value.A().SetValue(ControlValue::Max());
  BOOST_CHECK_EQUAL(value.A().Value().UInt(), ControlValue::MaxUInt());
  PresetCollection &presetCollection = *management.AddPresetCollectionPtr();
  presetCollection.SetFromCurrentSituation(management);
  BOOST_CHECK_EQUAL(presetCollection.PresetValues().size(), 1);
  BOOST_CHECK_EQUAL(&presetCollection.PresetValues()[0]->GetControllable(),
                    control);
  BOOST_CHECK_EQUAL(presetCollection.PresetValues()[0]->Value().UInt(),
                    ControlValue::MaxUInt());
}

BOOST_AUTO_TEST_CASE(SetValue) {
  const glight::system::Settings settings;
  Management management(settings);
  FixtureType &fixtureType =
      *management.GetTheatre().AddFixtureType(StockFixture::Light1Ch);
  Fixture &fixture =
      *management.GetTheatre().AddFixture(fixtureType.Modes().front());
  fixture.SetChannel(DmxChannel(100, 0));
  FixtureControl &fixtureControl = *management.AddFixtureControlPtr(fixture);
  BOOST_REQUIRE_EQUAL(fixtureControl.NInputs(), 1);
  SourceValue &value = management.AddSourceValue(fixtureControl, 0);
  value.A().SetValue(ControlValue::Max());
  PresetCollection &presetCollection = *management.AddPresetCollectionPtr();
  presetCollection.SetFromCurrentSituation(management);

  fixtureControl.InputValue(0) = ControlValue::Zero();
  presetCollection.InputValue(0) = ControlValue::Zero();
  presetCollection.MixInput(0, ControlValue::Max());

  std::vector<unsigned> values(512, 0);
  Timing timing(0.0, 0, 0, 0, 0);
  // Mix controls in order of dependencies
  presetCollection.Mix(timing, true);
  fixtureControl.Mix(timing, true);
  fixtureControl.GetChannelValues(values.data(), 0);
  for (size_t i = 0; i != 512; ++i) {
    if (i == 100) {
      // it's not accurately Max, because of truncations.
      BOOST_CHECK_GE(values[100], (ControlValue::MaxUInt() >> 8) * 255);
    } else
      BOOST_CHECK_EQUAL(values[i], 0);
  }
}

BOOST_AUTO_TEST_SUITE_END()
