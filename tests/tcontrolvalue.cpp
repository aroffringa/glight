#include "../theatre/controlvalue.h"

#include <boost/test/unit_test.hpp>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(control_value)

BOOST_AUTO_TEST_CASE(MinOperator) {
  const ControlValue a(3);
  const ControlValue b(4);
  const ControlValue c(5);
  const ControlValue d(6);
  BOOST_CHECK(Min(a, a) == a);
  BOOST_CHECK(Min(a, b) == a);
  BOOST_CHECK(Min(b, a) == a);
  BOOST_CHECK(Min(a, b, c) == a);
  BOOST_CHECK(Min(c, b, a) == a);
  BOOST_CHECK(Min(b, a, c) == a);
  BOOST_CHECK(Min(a, b, c, d) == a);
  BOOST_CHECK(Min(c, b, d, a) == a);
  BOOST_CHECK(Min(b, a, b, b) == a);
}

BOOST_AUTO_TEST_SUITE_END()
