#include "system/math.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(math)

BOOST_AUTO_TEST_CASE(radial_distance) {
  using glight::system::RadialDistance;
  const double result = RadialDistance(0.0, 0.0);
  BOOST_CHECK_LT(result, 1e-6);
  BOOST_CHECK_LT(RadialDistance(1.0, 1.0), 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialDistance(0.0, 1.0), 1.0, 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialDistance(1.0, 0.0), 1.0, 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialDistance(0.0, 2.0 * M_PI + 1.0), 1.0, 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialDistance(2.0 * M_PI + 1.0, 0.0), 1.0, 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialDistance(-2.0, 2.0 * M_PI - 1.0), 1.0, 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialDistance(2.0 * M_PI - 2.0, -1.0), 1.0, 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialDistance(-2.0, 8.0 * M_PI - 1.0), 1.0, 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialDistance(8.0 * M_PI - 2.0, -1.0), 1.0, 1e-6);
}

BOOST_AUTO_TEST_CASE(radial_clamp) {
  using glight::system::RadialClamp;
  const double result = RadialClamp(1.0, 0.0, 2.0);
  BOOST_CHECK_CLOSE_FRACTION(result, 1.0, 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialClamp(0.0, 1.0, 2.0), 1.0, 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialClamp(3.0, 1.0, 2.0), 2.0, 1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialClamp(1.5 - 2.0 * M_PI, 1.0, 2.0), 1.5,
                             1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialClamp(1.5 + 2.0 * M_PI, 1.0, 2.0), 1.5,
                             1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialClamp(1.5 - 4.0 * M_PI, -1.0, 2.0), 1.5,
                             1e-6);
  BOOST_CHECK_CLOSE_FRACTION(RadialClamp(1.5 + 4.0 * M_PI, -1.0, 2.0), 1.5,
                             1e-6);
}

BOOST_AUTO_TEST_SUITE_END()
