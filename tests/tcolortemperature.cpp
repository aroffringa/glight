#include "../system/colortemperature.h"

#include <boost/test/unit_test.hpp>

using glight::system::TemperatureToRgb;
using glight::theatre::Color;

BOOST_AUTO_TEST_SUITE(color_temperature)

BOOST_AUTO_TEST_CASE(warm) {
  const Color c = TemperatureToRgb(1500);
  BOOST_CHECK_EQUAL(255, c.Red());
  BOOST_CHECK_EQUAL(108, c.Green());
  BOOST_CHECK_EQUAL(0, c.Blue());
}

BOOST_AUTO_TEST_CASE(cold) {
  const Color c = TemperatureToRgb(15000);
  BOOST_CHECK_EQUAL(181, c.Red());
  BOOST_CHECK_EQUAL(205, c.Green());
  BOOST_CHECK_EQUAL(255, c.Blue());
}

BOOST_AUTO_TEST_SUITE_END()
