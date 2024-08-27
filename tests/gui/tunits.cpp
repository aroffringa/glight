#include "gui/units.h"

#include <boost/test/unit_test.hpp>

using namespace glight::gui;

BOOST_AUTO_TEST_SUITE(units)

BOOST_AUTO_TEST_CASE(angle_to_nice_string) {
  BOOST_CHECK_EQUAL(AngleToNiceString(0.0), "0.0");
  BOOST_CHECK_EQUAL(AngleToNiceString(3.03 * M_PI / 180.0), "3.0");
  BOOST_CHECK_EQUAL(AngleToNiceString(-3.03 * M_PI / 180.0), "-3.0");
  BOOST_CHECK_EQUAL(AngleToNiceString(3.06 * M_PI / 180.0), "3.1");
  BOOST_CHECK_EQUAL(AngleToNiceString(-3.06 * M_PI / 180.0), "-3.1");
}

BOOST_AUTO_TEST_SUITE_END()
