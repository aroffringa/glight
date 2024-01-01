#include "theatre/filters/monochromefilter.h"

#include <boost/test/unit_test.hpp>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(monochrome_filter)

BOOST_AUTO_TEST_CASE(types) {
  MonochromeFilter filter;
  filter.SetOutputTypes({FunctionType::Master, FunctionType::Red,
                         FunctionType::Green, FunctionType::Blue,
                         FunctionType::Strobe});
  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 3);
  BOOST_CHECK(filter.InputTypes()[0] == FunctionType::White);
  BOOST_CHECK(filter.InputTypes()[1] == FunctionType::Master);
  BOOST_CHECK(filter.InputTypes()[2] == FunctionType::Strobe);

  filter.SetOutputTypes({FunctionType::White});
  BOOST_REQUIRE_EQUAL(filter.InputTypes().size(), 1);
  BOOST_CHECK(filter.InputTypes()[0] == FunctionType::White);
}

BOOST_AUTO_TEST_CASE(apply) {
  MonochromeFilter filter;
  filter.SetOutputTypes({FunctionType::Master, FunctionType::Red,
                         FunctionType::Green, FunctionType::Blue,
                         FunctionType::Strobe});
  std::vector<ControlValue> output(5);
  filter.Apply(
      {
          ControlValue(10),  // white
          ControlValue(20),  // master
          ControlValue(30)   // strobe
      },
      output);
  BOOST_CHECK_EQUAL(output[0].UInt(), 20);  // master
  BOOST_CHECK_EQUAL(output[1].UInt(), 10);  // red
  BOOST_CHECK_EQUAL(output[2].UInt(), 10);  // green
  BOOST_CHECK_EQUAL(output[3].UInt(), 10);  // blue
  BOOST_CHECK_EQUAL(output[4].UInt(), 30);  // strobe
}

BOOST_AUTO_TEST_SUITE_END()
