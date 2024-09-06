#include "theatre/fixturetypefunction.h"

#include <boost/test/unit_test.hpp>

#include <memory>

namespace glight::theatre {

BOOST_AUTO_TEST_SUITE(fixture_type_function)

BOOST_AUTO_TEST_CASE(copy) {
  FixtureTypeFunction a(FunctionType::ColorMacro, 12,
                        system::OptionalNumber<size_t>(), 0);
  a.GetColorRangeParameters().GetRanges().emplace_back(100, 200, Color::Lime());
  a.GetColorRangeParameters().GetRanges().emplace_back(300, 500,
                                                       std::optional<Color>());
  FixtureTypeFunction b(FunctionType::White, 11,
                        system::OptionalNumber<size_t>(), 0);
  b = std::move(a);
  BOOST_CHECK(b.Type() == FunctionType::ColorMacro);
  BOOST_CHECK_EQUAL(b.DmxOffset(), 12);
  BOOST_CHECK(!b.FineChannelOffset());
  BOOST_CHECK_EQUAL(b.Shape(), 0);
  BOOST_REQUIRE_EQUAL(b.GetColorRangeParameters().GetRanges().size(), 2);
}

BOOST_AUTO_TEST_CASE(range_function) {
  FixtureTypeFunction f(FunctionType::RotationSpeed, 37, {}, 2);
  BOOST_CHECK_EQUAL(f.DmxOffset(), 37);
  BOOST_CHECK(f.Type() == FunctionType::RotationSpeed);
  BOOST_CHECK(!f.FineChannelOffset());
  BOOST_CHECK_EQUAL(f.Shape(), 2);

  std::vector<RotationSpeedParameters::Range>& ranges =
      f.GetRotationParameters().GetRanges();
  ranges.emplace_back(10, 110, 0, 100000);
  ranges.emplace_back(110, 210, 100000, 100100);
  ranges.emplace_back(210, 256, -46, 0);

  BOOST_CHECK_EQUAL(details::GetParameterRange(0, ranges), nullptr);
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(0), 0);
  BOOST_CHECK_EQUAL(details::GetParameterRange(1, ranges), nullptr);
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(1), 0);
  BOOST_CHECK_EQUAL(details::GetParameterRange(10, ranges), ranges.data());
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(10), 0);
  BOOST_CHECK_EQUAL(details::GetParameterRange(11, ranges), ranges.data());
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(11), 1000);
  BOOST_CHECK_EQUAL(details::GetParameterRange(110, ranges), &ranges[1]);
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(110), 100000);
  BOOST_CHECK_EQUAL(details::GetParameterRange(111, ranges), &ranges[1]);
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(111), 100001);
  BOOST_CHECK_EQUAL(details::GetParameterRange(210, ranges), &ranges[2]);
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(210), -46);
  BOOST_CHECK_EQUAL(details::GetParameterRange(255, ranges), &ranges[2]);
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(255), -1);
}

}  // namespace glight::theatre

BOOST_AUTO_TEST_SUITE_END()
