#include "../theatre/fixturecontrol.h"
#include "../theatre/folder.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"
#include "../theatre/timing.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(fixture_control)

BOOST_AUTO_TEST_CASE(SetValue) {
  Management management;
  FixtureType &fixtureType =
      management.GetTheatre().AddFixtureType(StockFixture::Light1Ch);
  Fixture &fixture = management.GetTheatre().AddFixture(fixtureType);
  fixture.SetChannel(100);
  FixtureControl &control = management.AddFixtureControl(fixture);
  control.InputValue(0) = ControlValue::Zero();
  control.MixInput(0, ControlValue::Max());
  std::vector<unsigned> values(512, 0);
  Timing timing(0.0, 0, 0, 0, 0);
  control.Mix(timing, true);
  control.MixChannels(values.data(), 0);
  for (size_t i = 0; i != 512; ++i) {
    if (i == 100)
      BOOST_CHECK_EQUAL(values[100], ControlValue::MaxUInt());
    else
      BOOST_CHECK_EQUAL(values[i], 0);
  }
}

BOOST_AUTO_TEST_SUITE_END()
