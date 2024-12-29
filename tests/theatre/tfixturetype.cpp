#include "theatre/fixture.h"
#include "theatre/fixturetype.h"
#include "theatre/management.h"
#include "theatre/theatre.h"

#include <boost/test/unit_test.hpp>

#include <vector>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(fixture_type)

BOOST_AUTO_TEST_CASE(ClassList) {
  const std::vector<FixtureClass> list = FixtureType::GetClassList();
  BOOST_REQUIRE(!list.empty());

  for (FixtureClass cl : list) {
    BOOST_CHECK(FixtureType::NameToClass(FixtureType::ClassName(cl)) == cl);
  }
  BOOST_CHECK_NO_THROW(
      FixtureType::ClassName((FixtureClass)std::numeric_limits<int>::max()));
  BOOST_CHECK_THROW(FixtureType::NameToClass("This is not a class! ~!@"),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(StockList) {
  const std::vector<StockFixture> list = FixtureType::GetStockList();
  BOOST_REQUIRE(!list.empty());

  for (StockFixture stock_fixture : list) {
    BOOST_CHECK(!FixtureType::StockName(stock_fixture).empty());
  }
  BOOST_CHECK_NO_THROW(
      FixtureType::StockName((StockFixture)std::numeric_limits<int>::max()));
}

BOOST_AUTO_TEST_CASE(Construct) {
  const std::vector<StockFixture> list = FixtureType::GetStockList();
  for (StockFixture cl : list) {
    FixtureType type(cl);
    BOOST_CHECK(!type.Functions().empty());
  }
}

BOOST_AUTO_TEST_CASE(Copy) {
  FixtureType typeA(StockFixture::Rgb3Ch);
  FixtureType typeB(typeA);
  BOOST_REQUIRE_EQUAL(typeB.Functions().size(), 3);
  BOOST_CHECK(typeB.Functions()[0].Type() == FunctionType::Red);
  BOOST_CHECK(typeB.Functions()[1].Type() == FunctionType::Green);
  BOOST_CHECK(typeB.Functions()[2].Type() == FunctionType::Blue);
}

BOOST_AUTO_TEST_CASE(ShapeCount) {
  FixtureType typeA(StockFixture::Rgb3Ch);
  BOOST_CHECK_EQUAL(typeA.ShapeCount(), 1);
  FixtureType typeB(StockFixture::BT_VINTAGE_5CH);
  BOOST_CHECK_EQUAL(typeB.ShapeCount(), 2);
}

Color testColor(StockFixture cl, const std::vector<unsigned char> &values) {
  Management management;
  const FixtureType &fixtureType = *management.GetTheatre().AddFixtureType(cl);
  Fixture &rgbFixture = *management.GetTheatre().AddFixture(fixtureType);
  const ValueSnapshot snapShot(true, 1);
  ValueUniverseSnapshot &uni = snapShot.GetUniverseSnapshot(0);
  uni.SetValues(values.data(), values.size());
  return rgbFixture.Type().GetColor(rgbFixture, snapShot, 0);
}

BOOST_AUTO_TEST_CASE(GetColor_RGB) {
  const Color color = testColor(StockFixture::Rgb3Ch, {17, 128, 255});
  BOOST_TEST(color.Red() == 17);
  BOOST_TEST(color.Green() == 128);
  BOOST_TEST(color.Blue() == 255);
}

BOOST_AUTO_TEST_CASE(GetColor_RGBAWUV) {
  const Color colorRed =
      testColor(StockFixture::RgbawUv6Ch, {255, 0, 0, 0, 0, 0});
  BOOST_TEST(colorRed.Red() >= 64);
  BOOST_TEST(colorRed.Green() == 0);
  BOOST_TEST(colorRed.Blue() == 0);

  const Color colorGreen =
      testColor(StockFixture::RgbawUv6Ch, {0, 255, 0, 0, 0, 0});
  BOOST_TEST(colorGreen.Red() == 0);
  BOOST_TEST(colorGreen.Green() >= 64);
  BOOST_TEST(colorGreen.Blue() == 0);

  const Color colorBlue =
      testColor(StockFixture::RgbawUv6Ch, {0, 0, 255, 0, 0, 0});
  BOOST_TEST(colorBlue.Red() == 0);
  BOOST_TEST(colorBlue.Green() == 0);
  BOOST_TEST(colorBlue.Blue() >= 64);

  const Color colorAmber =
      testColor(StockFixture::RgbawUv6Ch, {0, 0, 0, 255, 0, 0});
  BOOST_TEST(colorAmber.Red() >= 32);
  BOOST_TEST(colorAmber.Green() >= 24);
  BOOST_TEST(colorAmber.Blue() == 0);

  const Color colorWhite =
      testColor(StockFixture::RgbawUv6Ch, {0, 0, 0, 0, 255, 0});
  BOOST_TEST(colorWhite.Red() >= 64);
  BOOST_TEST(colorWhite.Green() >= 64);
  BOOST_TEST(colorWhite.Blue() >= 64);

  const Color colorUV =
      testColor(StockFixture::RgbawUv6Ch, {0, 0, 0, 0, 0, 255});
  BOOST_TEST(colorUV.Red() >= 24);
  BOOST_TEST(colorUV.Green() == 0);
  BOOST_TEST(colorUV.Blue() >= 48);
}

BOOST_AUTO_TEST_CASE(GetRotation_AyraTDCSunrise) {
  Management management;
  const FixtureType &fixtureType =
      *management.GetTheatre().AddFixtureType(StockFixture::AyraTDCSunrise);
  Fixture &fixture = *management.GetTheatre().AddFixture(fixtureType);
  const ValueSnapshot snapShot(true, 1);
  ValueUniverseSnapshot &uni = snapShot.GetUniverseSnapshot(0);
  // Master, R, G, B, Strobe, Rotation, Macro
  const std::vector<unsigned char> values{255, 255, 0, 0, 0, 128, 0};
  uni.SetValues(values.data(), values.size());
  const int speed = fixture.Type().GetRotationSpeed(fixture, snapShot, 0);
  BOOST_CHECK_EQUAL(speed, -((1 << 24) / 100));
}

BOOST_AUTO_TEST_CASE(function_summary) {
  Management management;

  const FixtureType &rgb_type =
      *management.GetTheatre().AddFixtureType(StockFixture::Rgb3Ch);
  BOOST_CHECK_EQUAL(FunctionSummary(rgb_type), "R-G-B");

  const FixtureType &btv_type =
      *management.GetTheatre().AddFixtureType(StockFixture::BT_VINTAGE_7CH);
  BOOST_CHECK_EQUAL(FunctionSummary(btv_type), "W-M-S-R-G-B-C");
}

BOOST_AUTO_TEST_SUITE_END()
