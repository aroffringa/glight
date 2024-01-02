#include "../theatre/fixturecontrol.h"
#include "../theatre/folder.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"
#include "../theatre/timing.h"

#include "../theatre/filters/automasterfilter.h"
#include "../theatre/filters/monochromefilter.h"
#include "../theatre/filters/rgbfilter.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(fixture_control)

BOOST_AUTO_TEST_CASE(SetValue) {
  Management management;
  FixtureType &fixtureType =
      management.GetTheatre().AddFixtureType(StockFixture::Light1Ch);
  Fixture &fixture = management.GetTheatre().AddFixture(fixtureType);
  fixture.SetChannel(DmxChannel(100, 0));
  FixtureControl &control = management.AddFixtureControl(fixture);
  BOOST_REQUIRE_EQUAL(control.NInputs(), 1);
  BOOST_CHECK_EQUAL(fixture.Functions().size(), 1);
  BOOST_CHECK_EQUAL(fixture.Functions().front()->MainChannel().Channel(), 100);
  BOOST_CHECK(!fixture.Functions().front()->FineChannel());
  control.InputValue(0) = ControlValue::Zero();
  control.MixInput(0, ControlValue::Max());
  std::vector<unsigned> values(512, 0);
  Timing timing(0.0, 0, 0, 0, 0);
  control.Mix(timing, true);
  control.GetChannelValues(values.data(), 0);
  for (size_t i = 0; i != 512; ++i) {
    if (i == 100)
      BOOST_CHECK_EQUAL(values[100], ControlValue::MaxUInt());
    else
      BOOST_CHECK_EQUAL(values[i], 0);
  }
}

BOOST_AUTO_TEST_CASE(Filters) {
  Management management;

  {
    FixtureType &fixtureType =
        management.GetTheatre().AddFixtureType(StockFixture::Rgba4Ch);
    Fixture &fixture = management.GetTheatre().AddFixture(fixtureType);
    FixtureControl &control = management.AddFixtureControl(fixture);
    control.AddFilter(std::make_unique<RgbFilter>());
    control.AddFilter(std::make_unique<MonochromeFilter>());
    BOOST_REQUIRE_EQUAL(control.NInputs(), 1);
    BOOST_CHECK(control.InputType(0) == FunctionType::White);
  }
  {
    FixtureType &fixtureType =
        management.GetTheatre().AddFixtureType(StockFixture::Rgba5Ch);
    Fixture &fixture = management.GetTheatre().AddFixture(fixtureType);
    FixtureControl &control = management.AddFixtureControl(fixture);
    control.AddFilter(std::make_unique<AutoMasterFilter>());
    control.AddFilter(std::make_unique<RgbFilter>());
    BOOST_REQUIRE_EQUAL(control.NInputs(), 3);
    BOOST_CHECK(control.InputType(0) == FunctionType::Red);
    BOOST_CHECK(control.InputType(1) == FunctionType::Green);
    BOOST_CHECK(control.InputType(2) == FunctionType::Blue);
  }
}

BOOST_AUTO_TEST_SUITE_END()
