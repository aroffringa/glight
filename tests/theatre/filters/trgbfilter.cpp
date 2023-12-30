#include "theatre/filters/rgbfilter.h"

#include <algorithm>

#include <boost/test/unit_test.hpp>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(rgb_filter)

namespace {
void ToleranceCheck(const ControlValue& value, unsigned expected,
                    unsigned tolerance = 1) {
  BOOST_CHECK_LE(
      std::abs(static_cast<int>(value.UInt()) - static_cast<int>(expected)),
      tolerance);
}
}  // namespace

constexpr unsigned kFull = ControlValue::MaxUInt();
constexpr ControlValue kZeroCV = ControlValue::Zero();
constexpr ControlValue kFullCV = ControlValue::Max();

BOOST_AUTO_TEST_CASE(types) {
  RgbFilter filter;
  filter.SetOutputTypes({
      FunctionType::Master,
      FunctionType::Red,
      FunctionType::Green,
      FunctionType::Blue,
      FunctionType::Amber,
      FunctionType::White,
      FunctionType::Strobe,
  });
  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 5);
  BOOST_CHECK(filter.InputTypes()[0] == FunctionType::Red);
  BOOST_CHECK(filter.InputTypes()[1] == FunctionType::Green);
  BOOST_CHECK(filter.InputTypes()[2] == FunctionType::Blue);
  BOOST_CHECK(filter.InputTypes()[3] == FunctionType::Master);
  BOOST_CHECK(filter.InputTypes()[4] == FunctionType::Strobe);

  filter.SetOutputTypes({FunctionType::White});
  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 3);
  BOOST_CHECK(filter.InputTypes()[0] == FunctionType::Red);
  BOOST_CHECK(filter.InputTypes()[1] == FunctionType::Green);
  BOOST_CHECK(filter.InputTypes()[2] == FunctionType::Blue);
}

BOOST_AUTO_TEST_CASE(white) {
  RgbFilter filter;
  filter.SetOutputTypes({
      FunctionType::Master,
      FunctionType::Red,
      FunctionType::Green,
      FunctionType::Blue,
      FunctionType::White,
      FunctionType::Strobe,
  });
  std::vector<ControlValue> output(5);
  filter.Apply(
      {
          kFullCV,            // red
          kFullCV,            // green
          kFullCV,            // blue
          ControlValue(42),   // master
          ControlValue(1982)  // strobe
      },
      output);
  BOOST_CHECK_EQUAL(output[0].UInt(), 42);     // master
  ToleranceCheck(output[1], kFull / 2);        // red
  ToleranceCheck(output[2], kFull / 2);        // green
  ToleranceCheck(output[3], kFull / 2);        // blue
  BOOST_CHECK_EQUAL(output[4].UInt(), kFull);  // white
  BOOST_CHECK_EQUAL(output[5].UInt(), 1982);   // strobe
}

BOOST_AUTO_TEST_CASE(cold_and_warm_white) {
  RgbFilter filter;
  filter.SetOutputTypes({
      FunctionType::WarmWhite,
      FunctionType::ColdWhite,
  });
  std::vector<ControlValue> output(2);

  // All full on
  filter.Apply({kFullCV, kFullCV, kFullCV}, output);
  BOOST_CHECK_EQUAL(output[0].UInt(), kFull);  // ww
  BOOST_CHECK_EQUAL(output[1].UInt(), kFull);  // cw

  // All full off
  filter.Apply({kZeroCV, kZeroCV, kZeroCV}, output);
  BOOST_CHECK_EQUAL(output[0].UInt(), 0);
  BOOST_CHECK_EQUAL(output[1].UInt(), 0);

  // Pastel red
  filter.Apply({kFullCV, ControlValue(kFull / 2), ControlValue(kFull / 2)},
               output);
  ToleranceCheck(output[0], kFull * 64 / (2 * 57));
  BOOST_CHECK_EQUAL(output[1].UInt(), 0);

  // Pastel blue
  filter.Apply({ControlValue(kFull / 2), ControlValue(kFull / 2), kFullCV},
               output);
  BOOST_CHECK_EQUAL(output[0].UInt(), 0);
  ToleranceCheck(output[1], kFull * 64 / (2 * 57));
}

BOOST_AUTO_TEST_SUITE_END()
