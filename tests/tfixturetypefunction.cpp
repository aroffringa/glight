
#include "../theatre/fixturetypefunction.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(fixture_type_function)

BOOST_AUTO_TEST_CASE(range_function) {
  FixtureTypeFunction f(37, FunctionType::Rotation, false, 2);
  BOOST_CHECK_EQUAL(f.DmxOffset(), 37);
  BOOST_CHECK(f.Type() == FunctionType::Rotation);
  BOOST_CHECK(!f.Is16Bit());
  BOOST_CHECK_EQUAL(f.Shape(), 2);

  std::vector<RotationParameters::Range>& ranges =
      f.GetRotationParameters().GetRanges();
  ranges.emplace_back(10, 110, 0, 100000);
  ranges.emplace_back(110, 210, 100000, 100100);
  ranges.emplace_back(210, 256, -46, 0);

  BOOST_CHECK_EQUAL(details::GetParameterRange(0, ranges), nullptr);
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(0), 0);
  BOOST_CHECK_EQUAL(details::GetParameterRange(1, ranges), nullptr);
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(1), 0);
  BOOST_CHECK_EQUAL(details::GetParameterRange(10, ranges), &ranges[0]);
  BOOST_CHECK_EQUAL(f.GetRotationParameters().GetSpeed(10), 0);
  BOOST_CHECK_EQUAL(details::GetParameterRange(11, ranges), &ranges[0]);
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

BOOST_AUTO_TEST_SUITE_END()
