#include <boost/test/unit_test.hpp>

#include <memory>

#include "system/uniquewithoutordering.h"

BOOST_AUTO_TEST_SUITE(unique_without_ordering)

BOOST_AUTO_TEST_CASE(empty) {
  const std::vector<unsigned> result = UniqueWithoutOrdering<unsigned>({});
  const std::vector<unsigned> empty;
  BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), empty.begin(),
                                empty.end());
}

BOOST_AUTO_TEST_CASE(simple) {
  const std::vector<char> result = UniqueWithoutOrdering<char>({'R'});
  const std::vector<char> expected = {'R'};
  BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(),
                                expected.end());
}

BOOST_AUTO_TEST_CASE(multiple) {
  const std::vector<char> result = UniqueWithoutOrdering<char>({'R', 'R'});
  const std::vector<char> expected = {'R'};
  BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(),
                                expected.end());
}

BOOST_AUTO_TEST_CASE(complex) {
  const std::vector<char> result = UniqueWithoutOrdering<char>(
      {'B', 'A', 'C', 'B', 'A', 'A', 'D', 'E', 'B', 'A', 'F'});
  const std::vector<char> expected = {'B', 'A', 'C', 'D', 'E', 'F'};
  BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), expected.begin(),
                                expected.end());
}

BOOST_AUTO_TEST_SUITE_END()
