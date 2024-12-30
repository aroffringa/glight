#include "theatre/fixturegroup.h"
#include "theatre/management.h"
#include "theatre/theatre.h"

#include <boost/test/unit_test.hpp>

#include <vector>

namespace glight {

using theatre::Fixture;
using theatre::FixtureGroup;
using theatre::Management;
using theatre::StockFixture;
using theatre::Theatre;

using system::ObservingPtr;
using system::TrackablePtr;

BOOST_AUTO_TEST_SUITE(fixture_group)

BOOST_AUTO_TEST_CASE(construct) {
  FixtureGroup group("g1");
  BOOST_CHECK(group.Empty());
  BOOST_CHECK_EQUAL(group.Size(), 0);
  BOOST_CHECK_EQUAL(group.Name(), "g1");
}

BOOST_AUTO_TEST_CASE(insert) {
  theatre::Theatre theatre;
  const theatre::FixtureType& type =
      *theatre.AddFixtureTypePtr(StockFixture::Rgb3Ch);
  ObservingPtr<Fixture> a = theatre.AddFixturePtr(type);
  ObservingPtr<Fixture> b = theatre.AddFixturePtr(type);
  FixtureGroup group("g1");
  group.Insert(a);
  group.Insert(b);
  BOOST_CHECK_EQUAL(group.Size(), 2);
  BOOST_CHECK(group.Fixtures()[0] == a || group.Fixtures()[1] == a);
  BOOST_CHECK(group.Fixtures()[0] == b || group.Fixtures()[1] == b);
}

BOOST_AUTO_TEST_CASE(remove) {
  theatre::Theatre theatre;
  const theatre::FixtureType& type =
      *theatre.AddFixtureType(StockFixture::Rgb3Ch);
  ObservingPtr<Fixture> a = theatre.AddFixturePtr(type);
  ObservingPtr<Fixture> b = theatre.AddFixturePtr(type);
  ObservingPtr<Fixture> c = theatre.AddFixturePtr(type);
  FixtureGroup group("g1");
  BOOST_CHECK(!group.Remove(a));
  group.Insert(a);
  BOOST_CHECK(!group.Remove(b));
  group.Insert(b);
  group.Insert(c);
  BOOST_CHECK(group.Remove(b));
  BOOST_CHECK_EQUAL(group.Size(), 2);
  BOOST_CHECK(!group.Remove(b));
  BOOST_CHECK(group.Fixtures()[0] == a || group.Fixtures()[1] == a);
  BOOST_CHECK(group.Fixtures()[0] == c || group.Fixtures()[1] == c);
  BOOST_CHECK(group.Remove(a));
  BOOST_CHECK_EQUAL(group.Size(), 1);
  BOOST_CHECK(group.Fixtures()[0] == c);
  BOOST_CHECK(!group.Remove(a));
  group.Insert(a);
  BOOST_CHECK(group.Remove(c));
  BOOST_CHECK(group.Fixtures()[0] == a);
  BOOST_CHECK(group.Remove(a));
  BOOST_CHECK(group.Empty());
  BOOST_CHECK(!group.Remove(a));
}

BOOST_AUTO_TEST_CASE(contains) {
  Theatre theatre;
  const theatre::FixtureType& type =
      *theatre.AddFixtureType(StockFixture::Rgb3Ch);
  ObservingPtr<Fixture> a = theatre.AddFixturePtr(type);
  ObservingPtr<Fixture> b = theatre.AddFixturePtr(type);
  ObservingPtr<Fixture> c = theatre.AddFixturePtr(type);
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
  const theatre::FixtureType& type =
      *theatre.AddFixtureType(StockFixture::Rgb3Ch);
  ObservingPtr<Fixture> a = theatre.AddFixturePtr(type);
  theatre.AddFixture(type);
  ObservingPtr<Fixture> c = theatre.AddFixturePtr(type);
  FixtureGroup& group =
      *management.AddFixtureGroup(management.RootFolder(), "newgroup");
  BOOST_CHECK_EQUAL(group.Name(), "newgroup");
  group.Insert(a);
  group.Insert(c);
  BOOST_REQUIRE_EQUAL(management.FixtureGroups().size(), 1);
  BOOST_CHECK(management.FixtureGroups()[0].Get() == &group);
  management.RemoveObject(group);
  BOOST_CHECK(management.FixtureGroups().empty());
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace glight
