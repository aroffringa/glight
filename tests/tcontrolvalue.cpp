#include "../theatre/controlvalue.h"

#include <boost/test/unit_test.hpp>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(control_value)

BOOST_AUTO_TEST_CASE(MinOperator) {
  constexpr ControlValue a(3);
  constexpr ControlValue b(4);
  constexpr ControlValue c(5);
  constexpr ControlValue d(6);
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

BOOST_AUTO_TEST_CASE(Fraction) {
  constexpr unsigned m = ControlValue::MaxUInt();
  BOOST_CHECK_EQUAL(ControlValue::Fraction(0, m), 0);
  BOOST_CHECK_EQUAL(ControlValue::Fraction(m, m), m);
}

BOOST_AUTO_TEST_SUITE_END()
