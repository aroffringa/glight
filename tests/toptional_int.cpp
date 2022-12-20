#include "../system/optional_int.h"

#include <boost/test/unit_test.hpp>

using glight::system::OptionalInt;

template <typename T>
void test_optional_int() {
  OptionalInt<T> o1;
  BOOST_CHECK(!o1);
  BOOST_CHECK(!o1.has_value());
  BOOST_CHECK_THROW(o1.value(), std::bad_optional_access);
  BOOST_CHECK_EQUAL(o1.value_or(3), 3);

  OptionalInt<T> o2(std::nullopt);
  BOOST_CHECK(!o2);
  BOOST_CHECK(!o2.has_value());
  BOOST_CHECK_THROW(o2.value(), std::bad_optional_access);
  BOOST_CHECK_EQUAL(o2.value_or(0), 0);

  OptionalInt<T> o3(0);
  BOOST_CHECK(o3);
  BOOST_CHECK(o3.has_value());
  BOOST_CHECK_EQUAL(o3.value(), 0);
  BOOST_CHECK_EQUAL(o3.value_or(3), 0);

  const OptionalInt<T> o4(std::numeric_limits<T>::max() - 1);
  BOOST_CHECK(o4);
  BOOST_CHECK(o4.has_value());
  BOOST_CHECK_EQUAL(o4.value(), std::numeric_limits<T>::max() - 1);
  BOOST_CHECK_EQUAL(o4.value_or(1), std::numeric_limits<T>::max() - 1);

  OptionalInt<T> o5(o4);
  BOOST_CHECK(o5);
  BOOST_CHECK(o5.has_value());
  BOOST_CHECK_EQUAL(o5.value(), std::numeric_limits<T>::max() - 1);
  BOOST_CHECK_EQUAL(o5.value_or(1), std::numeric_limits<T>::max() - 1);

  o5.swap(o3);
  BOOST_CHECK(o3.has_value());
  BOOST_CHECK_EQUAL(o3.value(), std::numeric_limits<T>::max() - 1);
  BOOST_CHECK(o5.has_value());
  BOOST_CHECK_EQUAL(o5.value(), 0);

  o5.swap(o1);
  BOOST_CHECK(!o5.has_value());
  BOOST_CHECK(o1.has_value());
  BOOST_CHECK_EQUAL(o1.value(), 0);

  o1.reset();
  BOOST_CHECK(!o1.has_value());

  o1 = 7;
  BOOST_CHECK(o1.has_value());
  BOOST_CHECK_EQUAL(o1.value(), 7);

  o1 = o2;
  BOOST_CHECK(!o1.has_value());
  BOOST_CHECK(!o2.has_value());

  o1 = o3;
  BOOST_CHECK(o1.has_value());
  BOOST_CHECK_EQUAL(o1.value(), std::numeric_limits<T>::max() - 1);
  BOOST_CHECK(o3.has_value());
  BOOST_CHECK_EQUAL(o3.value(), std::numeric_limits<T>::max() - 1);
}

BOOST_AUTO_TEST_SUITE(optional_int)

BOOST_AUTO_TEST_CASE(with_int) { test_optional_int<int>(); }

BOOST_AUTO_TEST_CASE(with_uint16) { test_optional_int<uint16_t>(); }

BOOST_AUTO_TEST_CASE(compare) {
  OptionalInt<int> o_unset_a;
  OptionalInt<int> o_unset_b;
  OptionalInt<int> o_3a(3);
  OptionalInt<int> o_3b(3);
  OptionalInt<int> o_7(7);
  BOOST_CHECK(o_unset_a == o_unset_a);
  BOOST_CHECK(o_unset_a == o_unset_b);
  BOOST_CHECK(o_3a == o_3a);
  BOOST_CHECK(o_3a == o_3b);
  BOOST_CHECK(!(o_3a == o_unset_a));
  BOOST_CHECK(!(o_3a == o_7));

  BOOST_CHECK(!(o_unset_a != o_unset_a));
  BOOST_CHECK(!(o_unset_a != o_unset_b));
  BOOST_CHECK(!(o_3a != o_3a));
  BOOST_CHECK(!(o_3a != o_3b));
  BOOST_CHECK(o_3a != o_unset_a);
  BOOST_CHECK(o_3a != o_7);

  BOOST_CHECK(!(o_unset_a < o_unset_a));
  BOOST_CHECK(!(o_unset_a < o_unset_b));
  BOOST_CHECK(!(o_3a < o_3a));
  BOOST_CHECK(!(o_3a < o_3b));
  BOOST_CHECK(o_unset_a < o_3a);
  BOOST_CHECK(o_3a < o_7);
  BOOST_CHECK(!(o_3a < o_unset_a));
  BOOST_CHECK(!(o_7 < o_3a));
}

BOOST_AUTO_TEST_SUITE_END()
