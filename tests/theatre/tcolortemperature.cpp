#include "system/colortemperature.h"

#include <boost/test/unit_test.hpp>

using glight::system::TemperatureToRgb;
using glight::theatre::Color;

BOOST_AUTO_TEST_SUITE(color_temperature)

BOOST_AUTO_TEST_CASE(very_warm) {
  const Color c = TemperatureToRgb(1400);
  BOOST_CHECK_EQUAL(255, c.Red());
  BOOST_CHECK_EQUAL(108, c.Green());
  BOOST_CHECK_EQUAL(0, c.Blue());
}

BOOST_AUTO_TEST_CASE(warm2700) {
  const Color c = TemperatureToRgb(2700);
  BOOST_CHECK_EQUAL(255, c.Red());
  BOOST_CHECK_EQUAL(170, c.Green());  // 0xaa
  BOOST_CHECK_EQUAL(95, c.Blue());    // 0x5f
}

BOOST_AUTO_TEST_CASE(warm3000) {
  const Color c = TemperatureToRgb(3000);
  BOOST_CHECK_EQUAL(255, c.Red());
  BOOST_CHECK_EQUAL(180, c.Green());  // 0xb4
  BOOST_CHECK_EQUAL(116, c.Blue());   // 0x74
}

BOOST_AUTO_TEST_CASE(warm3200) {
  const Color c = TemperatureToRgb(3200);
  BOOST_CHECK_EQUAL(255, c.Red());
  BOOST_CHECK_EQUAL(186, c.Green());  // 0xba
  BOOST_CHECK_EQUAL(129, c.Blue());   // 0x81
}

BOOST_AUTO_TEST_CASE(warm4000) {
  const Color c = TemperatureToRgb(4000);
  BOOST_CHECK_EQUAL(255, c.Red());
  BOOST_CHECK_EQUAL(208, c.Green());  // 0xd0
  BOOST_CHECK_EQUAL(170, c.Blue());   // 0xaa
}

BOOST_AUTO_TEST_CASE(warm4500) {
  const Color c = TemperatureToRgb(4500);
  BOOST_CHECK_EQUAL(255, c.Red());
  BOOST_CHECK_EQUAL(219, c.Green());  // 0xdb
  BOOST_CHECK_EQUAL(191, c.Blue());   // 0xbf
}

BOOST_AUTO_TEST_CASE(warm4800) {
  const Color c = TemperatureToRgb(4800);
  BOOST_CHECK_EQUAL(255, c.Red());
  BOOST_CHECK_EQUAL(226, c.Green());  // 0xe2
  BOOST_CHECK_EQUAL(202, c.Blue());   // 0xca
}

BOOST_AUTO_TEST_CASE(neutral) {
  const Color c = TemperatureToRgb(6500);
  BOOST_CHECK_EQUAL(255, c.Red());
  BOOST_CHECK_EQUAL(255, c.Green());
  BOOST_CHECK_EQUAL(255, c.Blue());
}

BOOST_AUTO_TEST_CASE(cold7000) {
  const Color c = TemperatureToRgb(7000);
  BOOST_CHECK_EQUAL(239, c.Red());    // 0xef
  BOOST_CHECK_EQUAL(240, c.Green());  // 0xf0
  BOOST_CHECK_EQUAL(255, c.Blue());
}

BOOST_AUTO_TEST_CASE(cold8500) {
  const Color c = TemperatureToRgb(8500);
  BOOST_CHECK_EQUAL(213, c.Red());    // 0xd5
  BOOST_CHECK_EQUAL(225, c.Green());  // 0xe1
  BOOST_CHECK_EQUAL(255, c.Blue());
}

BOOST_AUTO_TEST_CASE(cold10500) {
  const Color c = TemperatureToRgb(10500);
  BOOST_CHECK_EQUAL(197, c.Red());    // 0xc5
  BOOST_CHECK_EQUAL(215, c.Green());  // 0xd7
  BOOST_CHECK_EQUAL(255, c.Blue());
}

BOOST_AUTO_TEST_CASE(very_cold) {
  const Color c = TemperatureToRgb(15000);
  BOOST_CHECK_EQUAL(180, c.Red());    // 0xb4
  BOOST_CHECK_EQUAL(204, c.Green());  // 0xcc
  BOOST_CHECK_EQUAL(255, c.Blue());
}

BOOST_AUTO_TEST_SUITE_END()
