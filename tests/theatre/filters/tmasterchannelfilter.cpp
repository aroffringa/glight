#include "theatre/filters/masterchannelfilter.h"

#include <boost/test/unit_test.hpp>

#include "tolerance_check.h"

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(master_channel_filter)

BOOST_AUTO_TEST_CASE(types) {
  MasterChannelFilter filter;
  filter.SetOutputTypes({FunctionType::Red, FunctionType::Green,
                         FunctionType::Blue, FunctionType::Master,
                         FunctionType::Strobe});
  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 4);
  BOOST_CHECK(filter.InputTypes()[0] == FunctionType::Red);
  BOOST_CHECK(filter.InputTypes()[1] == FunctionType::Green);
  BOOST_CHECK(filter.InputTypes()[2] == FunctionType::Blue);
  BOOST_CHECK(filter.InputTypes()[3] == FunctionType::Strobe);

  filter.SetOutputTypes({FunctionType::White});
  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 1);
  BOOST_CHECK(filter.InputTypes()[0] == FunctionType::White);
}

BOOST_AUTO_TEST_CASE(apply_full_on) {
  MasterChannelFilter filter;
  filter.SetOutputTypes({FunctionType::Master, FunctionType::Red,
                         FunctionType::Green, FunctionType::Blue,
                         FunctionType::Strobe});
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
  MasterChannelFilter filter;
  filter.SetOutputTypes({FunctionType::Red, FunctionType::Green,
                         FunctionType::Blue, FunctionType::Master});
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
