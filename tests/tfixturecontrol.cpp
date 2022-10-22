#include "../theatre/fixturecontrol.h"
#include "../theatre/folder.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"
#include "../theatre/timing.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(fixture_control)

BOOST_AUTO_TEST_CASE(Add) {
  Management management;
  FixtureType &fixtureType =
      management.GetTheatre().AddFixtureType(StockFixture::RGBLight3Ch);
  Fixture &fixture = management.GetTheatre().AddFixture(fixtureType);
  FixtureControl &control = management.AddFixtureControl(fixture);
  BOOST_CHECK_EQUAL(&management.GetFixtureControl(fixture), &control);
  BOOST_CHECK_EQUAL(&control.Fixture(), &fixture);
  BOOST_CHECK_EQUAL(control.NInputs(), 3);
  BOOST_CHECK(control.InputColor(0) == Color(255, 0, 0));
  BOOST_CHECK(control.InputColor(1) == Color(0, 255, 0));
  BOOST_CHECK(control.InputColor(2) == Color(0, 0, 255));
  BOOST_CHECK_EQUAL(control.NOutputs(), 0);
  BOOST_CHECK(control.InputType(0) != control.InputType(1));
  BOOST_CHECK(control.InputType(1) != control.InputType(2));
}

BOOST_AUTO_TEST_CASE(AddMany) {
  Management management;
  FixtureType &fixtureType =
      management.GetTheatre().AddFixtureType(StockFixture::RGBLight3Ch);
  Fixture *fixture = &management.GetTheatre().AddFixture(fixtureType);
  BOOST_CHECK_EQUAL(fixture->Name(), "A");
  for (size_t i = 0; i != 30; ++i) {
    fixture = &management.GetTheatre().AddFixture(fixtureType);
    management.AddFixtureControl(*fixture);
  }
  fixture = &management.GetTheatre().AddFixture(fixtureType);
  BOOST_CHECK_EQUAL(fixture->Name(), "AF");
  BOOST_CHECK_EQUAL(management.GetTheatre().Fixtures().size(), 32);
  for (size_t i = 0; i != 26 * (26 + 1) - 32; ++i) {
    fixture = &management.GetTheatre().AddFixture(fixtureType);
    management.AddFixtureControl(*fixture);
  }
  BOOST_CHECK_EQUAL(fixture->Name(), "ZZ");
  BOOST_CHECK_EQUAL(management.GetTheatre().Fixtures().size(), 26 * (26 + 1));
  fixture = &management.GetTheatre().AddFixture(fixtureType);
  management.AddFixtureControl(*fixture);
  BOOST_CHECK_EQUAL(fixture->Name(), "AAA");
  BOOST_CHECK_EQUAL(management.GetTheatre().Fixtures().size(), 26 * 27 + 1);
}

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

BOOST_AUTO_TEST_CASE(Remove) {
  Management management;
  FixtureType &fixtureType =
      management.GetTheatre().AddFixtureType(StockFixture::RGBLight3Ch);
  management.RootFolder().Add(fixtureType);
  Fixture &fixture = management.GetTheatre().AddFixture(fixtureType);
  FixtureControl &control =
      management.AddFixtureControl(fixture, management.RootFolder());
  const std::vector<std::unique_ptr<FixtureFunction>> &functions =
      fixture.Functions();
  BOOST_CHECK_EQUAL(functions.size(), 3);
  for (size_t i = 0; i != functions.size(); ++i)
    management.AddSourceValue(control, i);
  management.RemoveFixture(fixture);
  BOOST_CHECK_EQUAL(management.GetTheatre().Fixtures().size(), 0);
  BOOST_CHECK_EQUAL(management.Controllables().size(), 0);
  BOOST_CHECK_EQUAL(management.SourceValues().size(), 0);
  BOOST_CHECK_EQUAL(management.RootFolder().Children().size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
