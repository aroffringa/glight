#include "theatre/filters/rgbfilter.h"

#include "tests/tolerance_check.h"
#include "tests/theatre/filters/outputexamples.h"

#include <algorithm>

#include <boost/test/unit_test.hpp>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(rgb_filter)

constexpr unsigned kFull = ControlValue::MaxUInt();
constexpr ControlValue kZeroCV = ControlValue::Zero();
constexpr ControlValue kHalfCV = ControlValue::Max() / 2;
constexpr ControlValue kFullCV = ControlValue::Max();

BOOST_AUTO_TEST_CASE(types) {
  RgbFilter filter;
  filter.SetOutputTypes(MakeFunctionList({
      FunctionType::Master,
      FunctionType::Red,
      FunctionType::Green,
      FunctionType::Blue,
      FunctionType::Amber,
      FunctionType::White,
      FunctionType::Strobe,
  }));
  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 5);
  BOOST_CHECK(filter.InputTypes()[0].Type() == FunctionType::Red);
  BOOST_CHECK(filter.InputTypes()[1].Type() == FunctionType::Green);
  BOOST_CHECK(filter.InputTypes()[2].Type() == FunctionType::Blue);
  BOOST_CHECK(filter.InputTypes()[3].Type() == FunctionType::Master);
  BOOST_CHECK(filter.InputTypes()[4].Type() == FunctionType::Strobe);

  filter.SetOutputTypes(GetWhiteFunctionExample());
  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 3);
  BOOST_CHECK(filter.InputTypes()[0].Type() == FunctionType::Red);
  BOOST_CHECK(filter.InputTypes()[1].Type() == FunctionType::Green);
  BOOST_CHECK(filter.InputTypes()[2].Type() == FunctionType::Blue);
}

BOOST_AUTO_TEST_CASE(white) {
  RgbFilter filter;
  filter.SetOutputTypes(MakeFunctionList({
      FunctionType::Master,
      FunctionType::Red,
      FunctionType::Green,
      FunctionType::Blue,
      FunctionType::White,
      FunctionType::Strobe,
  }));
  std::vector<ControlValue> output(6);
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
  filter.SetOutputTypes(MakeFunctionList({
      FunctionType::WarmWhite,
      FunctionType::ColdWhite,
  }));
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

BOOST_AUTO_TEST_CASE(macro) {
  RgbFilter filter;
  std::vector<FixtureTypeFunction> functions =
      MakeFunctionList({FunctionType::ColorMacro});
  std::vector<ColorRangeParameters::Range>& ranges =
      functions.back().GetColorRangeParameters().GetRanges();
  ranges.emplace_back(0, 16, std::optional<Color>());
  ranges.emplace_back(16, 32, Color::RedC());
  ranges.emplace_back(32, 33, Color::GreenC());
  ranges.emplace_back(33, 256, Color::Amber());
  filter.SetOutputTypes(std::move(functions));

  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 3);

  std::vector<ControlValue> output(1);
  filter.Apply({kZeroCV, kZeroCV, kZeroCV}, output);
  BOOST_CHECK_EQUAL(output[0].ToUChar(), 8);
  filter.Apply({kFullCV, kZeroCV, kZeroCV}, output);
  BOOST_CHECK_EQUAL(output[0].ToUChar(), 24);
  filter.Apply({kZeroCV, ControlValue(kFull * 3 / 4), kZeroCV}, output);
  BOOST_CHECK_EQUAL(output[0].ToUChar(), 32);
  filter.Apply({kFullCV, kFullCV, kZeroCV}, output);
  BOOST_CHECK_EQUAL(output[0].ToUChar(), 144);
}

BOOST_AUTO_TEST_CASE(macro_with_master) {
  RgbFilter filter;
  std::vector<FixtureTypeFunction> functions =
      MakeFunctionList({FunctionType::ColorMacro, FunctionType::Master});
  std::vector<ColorRangeParameters::Range>& ranges =
      functions.front().GetColorRangeParameters().GetRanges();
  ranges.emplace_back(0, 16, std::optional<Color>());
  ranges.emplace_back(16, 32, Color::RedC());
  ranges.emplace_back(32, 33, Color::GreenC());
  ranges.emplace_back(33, 256, Color::Amber());
  filter.SetOutputTypes(std::move(functions));

  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 3);

  std::vector<ControlValue> output(2);

  filter.Apply({kZeroCV, kZeroCV, kZeroCV}, output);
  BOOST_CHECK_EQUAL(output[0].ToUChar(), 8);
  BOOST_CHECK_EQUAL(output[1].ToUChar(), 0);

  filter.Apply({kHalfCV, kZeroCV, kZeroCV}, output);
  BOOST_CHECK_EQUAL(output[0].ToUChar(), 24);
  BOOST_CHECK_EQUAL(output[1].ToUChar(), 127);

  filter.Apply({kZeroCV, ControlValue(kFull * 3 / 4), kZeroCV}, output);
  BOOST_CHECK_EQUAL(output[0].ToUChar(), 32);
  BOOST_CHECK_EQUAL(output[1].ToUChar(), 191);

  filter.Apply({kFullCV, kFullCV, kZeroCV}, output);
  BOOST_CHECK_EQUAL(output[0].ToUChar(), 144);
  BOOST_CHECK_EQUAL(output[1].ToUChar(), 255);
}

BOOST_AUTO_TEST_SUITE_END()
