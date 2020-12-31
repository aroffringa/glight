#include "../theatre/fixture.h"
#include "../theatre/fixturetype.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"

#include <boost/test/unit_test.hpp>

#include <vector>

BOOST_AUTO_TEST_SUITE(fixture_type)

BOOST_AUTO_TEST_CASE(ClassList) {
  const std::vector<enum FixtureType::FixtureClass> list =
      FixtureType::GetClassList();
  BOOST_REQUIRE(!list.empty());

  for (enum FixtureType::FixtureClass cl : list) {
    BOOST_CHECK_EQUAL(FixtureType::NameToClass(FixtureType::ClassName(cl)), cl);
  }
  BOOST_CHECK_NO_THROW(FixtureType::ClassName(
      (enum FixtureType::FixtureClass)std::numeric_limits<int>::max()));
  BOOST_CHECK_THROW(FixtureType::NameToClass("This is not a class! ~!@"),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Construct) {
  const std::vector<enum FixtureType::FixtureClass> list =
      FixtureType::GetClassList();
  for (enum FixtureType::FixtureClass cl : list) {
    FixtureType type(cl);
    BOOST_CHECK_EQUAL(type.FixtureClass(), cl);
    BOOST_CHECK(!type.FunctionTypes().empty());
  }
}

BOOST_AUTO_TEST_CASE(Copy) {
  FixtureType typeA(FixtureType::RGBLight3Ch);
  FixtureType typeB(typeA);
  BOOST_REQUIRE_EQUAL(typeB.FunctionTypes().size(), 3);
  BOOST_CHECK(typeB.FunctionTypes()[0] == FunctionType::Red);
  BOOST_CHECK(typeB.FunctionTypes()[1] == FunctionType::Green);
  BOOST_CHECK(typeB.FunctionTypes()[2] == FunctionType::Blue);
}

BOOST_AUTO_TEST_CASE(ShapeCount) {
  FixtureType typeA(FixtureType::RGBLight3Ch);
  BOOST_CHECK_EQUAL(typeA.ShapeCount(), 1);
  FixtureType typeB(FixtureType::BT_VINTAGE_5CH);
  BOOST_CHECK_EQUAL(typeB.ShapeCount(), 2);
}

Color testColor(enum FixtureType::FixtureClass cl,
                const std::vector<unsigned char> &values) {
  Management management;
  const FixtureType &fixtureType = management.Theatre().AddFixtureType(cl);
  Fixture &rgbFixture = management.Theatre().AddFixture(fixtureType);
  const ValueSnapshot snapShot(1);
  ValueUniverseSnapshot &uni = snapShot.GetUniverseSnapshot(0);
  uni.SetValues(values.data(), values.size());
  return rgbFixture.Type().GetColor(rgbFixture, snapShot, 0);
}

BOOST_AUTO_TEST_CASE(GetColor_RGB) {
  const Color color = testColor(FixtureType::RGBLight3Ch, {17, 128, 255});
  BOOST_TEST(color.Red() == 17);
  BOOST_TEST(color.Green() == 128);
  BOOST_TEST(color.Blue() == 255);
}

BOOST_AUTO_TEST_CASE(GetColor_RGBAWUV) {
  const Color colorRed =
      testColor(FixtureType::RGBAWUVLight6Ch, {255, 0, 0, 0, 0, 0});
  BOOST_TEST(colorRed.Red() >= 64);
  BOOST_TEST(colorRed.Green() == 0);
  BOOST_TEST(colorRed.Blue() == 0);

  const Color colorGreen =
      testColor(FixtureType::RGBAWUVLight6Ch, {0, 255, 0, 0, 0, 0});
  BOOST_TEST(colorGreen.Red() == 0);
  BOOST_TEST(colorGreen.Green() >= 64);
  BOOST_TEST(colorGreen.Blue() == 0);

  const Color colorBlue =
      testColor(FixtureType::RGBAWUVLight6Ch, {0, 0, 255, 0, 0, 0});
  BOOST_TEST(colorBlue.Red() == 0);
  BOOST_TEST(colorBlue.Green() == 0);
  BOOST_TEST(colorBlue.Blue() >= 64);

  const Color colorAmber =
      testColor(FixtureType::RGBAWUVLight6Ch, {0, 0, 0, 255, 0, 0});
  BOOST_TEST(colorAmber.Red() >= 32);
  BOOST_TEST(colorAmber.Green() >= 24);
  BOOST_TEST(colorAmber.Blue() == 0);

  const Color colorWhite =
      testColor(FixtureType::RGBAWUVLight6Ch, {0, 0, 0, 0, 255, 0});
  BOOST_TEST(colorWhite.Red() >= 64);
  BOOST_TEST(colorWhite.Green() >= 64);
  BOOST_TEST(colorWhite.Blue() >= 64);

  const Color colorUV =
      testColor(FixtureType::RGBAWUVLight6Ch, {0, 0, 0, 0, 0, 255});
  BOOST_TEST(colorUV.Red() >= 24);
  BOOST_TEST(colorUV.Green() == 0);
  BOOST_TEST(colorUV.Blue() >= 48);
}

BOOST_AUTO_TEST_SUITE_END()
