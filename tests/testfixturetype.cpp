#include "../theatre/fixture.h"
#include "../theatre/fixturetype.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"

#include <boost/test/unit_test.hpp>

#include <vector>

BOOST_AUTO_TEST_SUITE(fixture_type)

BOOST_AUTO_TEST_CASE(ClassList) {
  const std::vector<StockFixture> list = FixtureType::GetClassList();
  BOOST_REQUIRE(!list.empty());

  for (StockFixture cl : list) {
    BOOST_CHECK(FixtureType::NameToClass(FixtureType::ClassName(cl)) == cl);
  }
  BOOST_CHECK_NO_THROW(
      FixtureType::ClassName((StockFixture)std::numeric_limits<int>::max()));
  BOOST_CHECK_THROW(FixtureType::NameToClass("This is not a class! ~!@"),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(Construct) {
  const std::vector<StockFixture> list = FixtureType::GetClassList();
  for (StockFixture cl : list) {
    FixtureType type(cl);
    BOOST_CHECK(type.GetFixtureClass() == cl);
    BOOST_CHECK(!type.Functions().empty());
  }
}

BOOST_AUTO_TEST_CASE(Copy) {
  FixtureType typeA(StockFixture::RGBLight3Ch);
  FixtureType typeB(typeA);
  BOOST_REQUIRE_EQUAL(typeB.Functions().size(), 3);
  BOOST_CHECK(typeB.Functions()[0].type == FunctionType::Red);
  BOOST_CHECK(typeB.Functions()[1].type == FunctionType::Green);
  BOOST_CHECK(typeB.Functions()[2].type == FunctionType::Blue);
}

BOOST_AUTO_TEST_CASE(ShapeCount) {
  FixtureType typeA(StockFixture::RGBLight3Ch);
  BOOST_CHECK_EQUAL(typeA.ShapeCount(), 1);
  FixtureType typeB(StockFixture::BT_VINTAGE_5CH);
  BOOST_CHECK_EQUAL(typeB.ShapeCount(), 2);
}

Color testColor(StockFixture cl, const std::vector<unsigned char> &values) {
  Management management;
  const FixtureType &fixtureType = management.GetTheatre().AddFixtureType(cl);
  Fixture &rgbFixture = management.GetTheatre().AddFixture(fixtureType);
  const ValueSnapshot snapShot(1);
  ValueUniverseSnapshot &uni = snapShot.GetUniverseSnapshot(0);
  uni.SetValues(values.data(), values.size());
  return rgbFixture.Type().GetColor(rgbFixture, snapShot, 0);
}

BOOST_AUTO_TEST_CASE(GetColor_RGB) {
  const Color color = testColor(StockFixture::RGBLight3Ch, {17, 128, 255});
  BOOST_TEST(color.Red() == 17);
  BOOST_TEST(color.Green() == 128);
  BOOST_TEST(color.Blue() == 255);
}

BOOST_AUTO_TEST_CASE(GetColor_RGBAWUV) {
  const Color colorRed =
      testColor(StockFixture::RGBAWUVLight6Ch, {255, 0, 0, 0, 0, 0});
  BOOST_TEST(colorRed.Red() >= 64);
  BOOST_TEST(colorRed.Green() == 0);
  BOOST_TEST(colorRed.Blue() == 0);

  const Color colorGreen =
      testColor(StockFixture::RGBAWUVLight6Ch, {0, 255, 0, 0, 0, 0});
  BOOST_TEST(colorGreen.Red() == 0);
  BOOST_TEST(colorGreen.Green() >= 64);
  BOOST_TEST(colorGreen.Blue() == 0);

  const Color colorBlue =
      testColor(StockFixture::RGBAWUVLight6Ch, {0, 0, 255, 0, 0, 0});
  BOOST_TEST(colorBlue.Red() == 0);
  BOOST_TEST(colorBlue.Green() == 0);
  BOOST_TEST(colorBlue.Blue() >= 64);

  const Color colorAmber =
      testColor(StockFixture::RGBAWUVLight6Ch, {0, 0, 0, 255, 0, 0});
  BOOST_TEST(colorAmber.Red() >= 32);
  BOOST_TEST(colorAmber.Green() >= 24);
  BOOST_TEST(colorAmber.Blue() == 0);

  const Color colorWhite =
      testColor(StockFixture::RGBAWUVLight6Ch, {0, 0, 0, 0, 255, 0});
  BOOST_TEST(colorWhite.Red() >= 64);
  BOOST_TEST(colorWhite.Green() >= 64);
  BOOST_TEST(colorWhite.Blue() >= 64);

  const Color colorUV =
      testColor(StockFixture::RGBAWUVLight6Ch, {0, 0, 0, 0, 0, 255});
  BOOST_TEST(colorUV.Red() >= 24);
  BOOST_TEST(colorUV.Green() == 0);
  BOOST_TEST(colorUV.Blue() >= 48);
}

BOOST_AUTO_TEST_SUITE_END()
