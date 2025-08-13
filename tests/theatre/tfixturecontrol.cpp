#include "system/settings.h"

#include "theatre/fixturecontrol.h"
#include "theatre/fixturetype.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/theatre.h"
#include "theatre/timing.h"

#include "theatre/filters/automasterfilter.h"
#include "theatre/filters/monochromefilter.h"
#include "theatre/filters/rgbfilter.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;
using glight::system::ObservingPtr;

BOOST_AUTO_TEST_SUITE(fixture_control)

BOOST_AUTO_TEST_CASE(SetValue) {
  const glight::system::Settings settings;
  Management management(settings);
  ObservingPtr<FixtureType> fixture_type =
      management.GetTheatre().AddFixtureTypePtr(StockFixture::Light1Ch);
  BOOST_REQUIRE_EQUAL(fixture_type->Modes().size(), 1);
  BOOST_REQUIRE_EQUAL(fixture_type->Modes().front().Functions().size(), 1);
  Fixture &fixture = *management.GetTheatre().AddFixture(fixture_type->Modes().front());
  BOOST_REQUIRE_EQUAL(fixture.Functions().size(), 1);
  fixture.SetChannel(DmxChannel(100, 0));
  ObservingPtr<FixtureControl> control =
      management.AddFixtureControlPtr(fixture);
  BOOST_REQUIRE_EQUAL(control->NInputs(), 1);
  BOOST_CHECK_EQUAL(fixture.Functions().size(), 1);
  BOOST_CHECK_EQUAL(fixture.Functions().front()->MainChannel().Channel(), 100);
  BOOST_CHECK(!fixture.Functions().front()->FineChannel());
  control->InputValue(0) = ControlValue::Zero();
  control->MixInput(0, ControlValue::Max());
  std::vector<unsigned> values(512, 0);
  Timing timing(0.0, 0, 0, 0, 0);
  control->Mix(timing, true);
  control->GetChannelValues(values.data(), 0);
  for (size_t i = 0; i != 512; ++i) {
    if (i == 100)
      BOOST_CHECK_EQUAL(values[100], ControlValue::MaxUInt());
    else
      BOOST_CHECK_EQUAL(values[i], 0);
  }
}

BOOST_AUTO_TEST_CASE(Filters) {
  const glight::system::Settings settings;
  Management management(settings);

  {
    ObservingPtr<FixtureType> fixtureType =
        management.GetTheatre().AddFixtureTypePtr(StockFixture::Rgba4Ch);
    const FixtureMode& mode = fixtureType->Modes().front();
    Fixture &fixture = *management.GetTheatre().AddFixture(mode);
    ObservingPtr<FixtureControl> control =
        management.AddFixtureControlPtr(fixture);
    control->AddFilter(std::make_unique<RgbFilter>());
    control->AddFilter(std::make_unique<MonochromeFilter>());
    BOOST_REQUIRE_EQUAL(control->NInputs(), 1);
    BOOST_CHECK(control->InputType(0) == FunctionType::White);
  }
  {
    ObservingPtr<FixtureType> fixtureType =
        management.GetTheatre().AddFixtureTypePtr(StockFixture::Rgba5Ch);
    const FixtureMode& mode = fixtureType->Modes().front();
    Fixture &fixture = *management.GetTheatre().AddFixture(mode);
    ObservingPtr<FixtureControl> control =
        management.AddFixtureControlPtr(fixture);
    control->AddFilter(std::make_unique<AutoMasterFilter>());
    control->AddFilter(std::make_unique<RgbFilter>());
    BOOST_REQUIRE_EQUAL(control->NInputs(), 3);
    BOOST_CHECK(control->InputType(0) == FunctionType::Red);
    BOOST_CHECK(control->InputType(1) == FunctionType::Green);
    BOOST_CHECK(control->InputType(2) == FunctionType::Blue);
  }
}

BOOST_AUTO_TEST_SUITE_END()
