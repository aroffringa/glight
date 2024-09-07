#include "theatre/filters/automasterfilter.h"

#include <boost/test/unit_test.hpp>

#include "tests/tolerance_check.h"
#include "tests/theatre/filters/outputexamples.h"

namespace glight::theatre {

BOOST_AUTO_TEST_SUITE(auto_master_filter)

BOOST_AUTO_TEST_CASE(types) {
  AutoMasterFilter filter;
  filter.SetOutputTypes(GetRGBMSFunctionsExample());
  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 4);
  BOOST_CHECK(filter.InputTypes()[0].Type() == FunctionType::Red);
  BOOST_CHECK(filter.InputTypes()[1].Type() == FunctionType::Green);
  BOOST_CHECK(filter.InputTypes()[2].Type() == FunctionType::Blue);
  BOOST_CHECK(filter.InputTypes()[3].Type() == FunctionType::Strobe);

  filter.SetOutputTypes(GetWhiteFunctionExample());
  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 1);
  BOOST_CHECK(filter.InputTypes()[0].Type() == FunctionType::White);
}

BOOST_AUTO_TEST_CASE(apply_full_on) {
  AutoMasterFilter filter;
  filter.SetOutputTypes(GetMRGBSFunctionsExample());
  std::vector<ControlValue> output(5);
  filter.Apply(
      {
          ControlValue::Max() / 2,  // red
          ControlValue::Max(),      // green
          ControlValue::Max() / 4,  // blue
          ControlValue::Zero()      // strobe
      },
      output);
  BOOST_CHECK_EQUAL(output[0].UInt(), ControlValue::MaxUInt());      // master
  BOOST_CHECK_EQUAL(output[1].UInt(), ControlValue::MaxUInt() / 2);  // red
  BOOST_CHECK_EQUAL(output[2].UInt(), ControlValue::MaxUInt());      // green
  BOOST_CHECK_EQUAL(output[3].UInt(), ControlValue::MaxUInt() / 4);  // blue
  BOOST_CHECK_EQUAL(output[4].UInt(), 0);                            // strobe
}

BOOST_AUTO_TEST_CASE(apply_low_brightness) {
  AutoMasterFilter filter;
  filter.SetOutputTypes(GetRGBMFunctionsExample());
  std::vector<ControlValue> output(5);
  filter.Apply(
      {
          ControlValue::Max() / 64,   // red
          ControlValue::Max() / 128,  // green
          ControlValue::Max() / 128   // blue
      },
      output);
  ToleranceCheck(output[0], ControlValue::MaxUInt() / 8, 128);   // red
  ToleranceCheck(output[1], ControlValue::MaxUInt() / 16, 128);  // green
  ToleranceCheck(output[2], ControlValue::MaxUInt() / 16, 128);  // blue
  ToleranceCheck(output[3], ControlValue::MaxUInt() / 8, 128);   // master

  ToleranceCheck(output[3] * output[0], ControlValue::MaxUInt() / 64,
                 128);  // master * red
  ToleranceCheck(output[3] * output[1], ControlValue::MaxUInt() / 128,
                 128);  // master * green
  ToleranceCheck(output[3] * output[2], ControlValue::MaxUInt() / 128,
                 128);  // master * blue
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace glight::theatre
