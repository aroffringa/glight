#include "../theatre/fixturegroup.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"

#include <boost/test/unit_test.hpp>

#include <vector>

using glight::theatre::Fixture;
using glight::theatre::FixtureGroup;
using glight::theatre::Management;
using glight::theatre::StockFixture;
using glight::theatre::Theatre;

BOOST_AUTO_TEST_SUITE(fixture_group)

BOOST_AUTO_TEST_CASE(construct) {
  FixtureGroup group("g1");
  BOOST_CHECK(group.Empty());
  BOOST_CHECK_EQUAL(group.Size(), 0);
  BOOST_CHECK_EQUAL(group.Name(), "g1");
}

BOOST_AUTO_TEST_CASE(insert) {
  glight::theatre::Theatre theatre;
  const glight::theatre::FixtureType& type =
      theatre.AddFixtureType(StockFixture::Rgb3Ch);
  Fixture& a = theatre.AddFixture(type);
  Fixture& b = theatre.AddFixture(type);
  FixtureGroup group("g1");
  group.Insert(a);
  group.Insert(b);
  BOOST_CHECK_EQUAL(group.Size(), 2);
  BOOST_CHECK(group.Fixtures()[0] == &a || group.Fixtures()[1] == &a);
  BOOST_CHECK(group.Fixtures()[0] == &b || group.Fixtures()[1] == &b);
}

BOOST_AUTO_TEST_CASE(remove) {
  glight::theatre::Theatre theatre;
  const glight::theatre::FixtureType& type =
      theatre.AddFixtureType(StockFixture::Rgb3Ch);
  Fixture& a = theatre.AddFixture(type);
  Fixture& b = theatre.AddFixture(type);
  Fixture& c = theatre.AddFixture(type);
  FixtureGroup group("g1");
  BOOST_CHECK(!group.Remove(a));
  group.Insert(a);
  BOOST_CHECK(!group.Remove(b));
  group.Insert(b);
  group.Insert(c);
  BOOST_CHECK(group.Remove(b));
  BOOST_CHECK_EQUAL(group.Size(), 2);
  BOOST_CHECK(!group.Remove(b));
  BOOST_CHECK(group.Fixtures()[0] == &a || group.Fixtures()[1] == &a);
  BOOST_CHECK(group.Fixtures()[0] == &c || group.Fixtures()[1] == &c);
  BOOST_CHECK(group.Remove(a));
  BOOST_CHECK_EQUAL(group.Size(), 1);
  BOOST_CHECK(group.Fixtures()[0] == &c);
  BOOST_CHECK(!group.Remove(a));
  group.Insert(a);
  BOOST_CHECK(group.Remove(c));
  BOOST_CHECK(group.Fixtures()[0] == &a);
  BOOST_CHECK(group.Remove(a));
  BOOST_CHECK(group.Empty());
  BOOST_CHECK(!group.Remove(a));
}

BOOST_AUTO_TEST_CASE(contains) {
  Theatre theatre;
  const glight::theatre::FixtureType& type =
      theatre.AddFixtureType(StockFixture::Rgb3Ch);
  Fixture& a = theatre.AddFixture(type);
  Fixture& b = theatre.AddFixture(type);
  Fixture& c = theatre.AddFixture(type);
  FixtureGroup group("g1");
  BOOST_CHECK(!group.Contains(a));
  group.Insert(a);
  BOOST_CHECK(group.Contains(a));
  BOOST_CHECK(!group.Contains(b));
  group.Insert(b);
  group.Insert(c);
  BOOST_CHECK(group.Contains(c));
}

BOOST_AUTO_TEST_CASE(add_to_management) {
  Management management;
  Theatre& theatre = management.GetTheatre();
  const glight::theatre::FixtureType& type =
      theatre.AddFixtureType(StockFixture::Rgb3Ch);
  Fixture& a = theatre.AddFixture(type);
  theatre.AddFixture(type);
  Fixture& c = theatre.AddFixture(type);
  FixtureGroup& group =
      management.AddFixtureGroup(management.RootFolder(), "newgroup");
  BOOST_CHECK_EQUAL(group.Name(), "newgroup");
  group.Insert(a);
  group.Insert(c);
  BOOST_REQUIRE_EQUAL(management.FixtureGroups().size(), 1);
  BOOST_CHECK(management.FixtureGroups()[0].get() == &group);
  management.RemoveObject(group);
  BOOST_CHECK(management.FixtureGroups().empty());
}

BOOST_AUTO_TEST_SUITE_END()
