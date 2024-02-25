#include "theatre/valuesnapshot.h"

#include <boost/test/unit_test.hpp>

namespace glight {

using theatre::DmxChannel;
using theatre::ValueSnapshot;
  
BOOST_AUTO_TEST_SUITE(value_snapshot)

BOOST_AUTO_TEST_CASE(basic) {
  BOOST_CHECK_EQUAL(ValueSnapshot().UniverseCount(), 0);
  
  ValueSnapshot v1(true, 2);
  BOOST_CHECK_EQUAL(v1.UniverseCount(), 2);
  std::array<unsigned char, 10> values;
  std::fill(values.begin(), values.end(), false);
  values[5] = 255;
  values[6] = 42;
  v1.GetUniverseSnapshot(0).SetValues(values.data(), values.size());
  v1.GetUniverseSnapshot(0).SetValues(values.data()+2, 5);
  BOOST_CHECK_EQUAL(v1.GetUniverseSnapshot(0).GetValue(5), 255);
  BOOST_CHECK_EQUAL(v1.GetUniverseSnapshot(0).GetValue(6), 42);
  BOOST_CHECK_EQUAL(v1.GetValue(DmxChannel(5, 0)), 255);
  BOOST_CHECK_EQUAL(v1.GetValue(DmxChannel(6, 0)), 42);
  
  ValueSnapshot v2(v1);
  BOOST_CHECK_EQUAL(v2.UniverseCount(), 2);
  BOOST_CHECK_EQUAL(v2.GetValue(DmxChannel(5, 0)), 255);
  BOOST_CHECK_EQUAL(v2.GetValue(DmxChannel(6, 0)), 42);
  
  ValueSnapshot v3;
  BOOST_CHECK_EQUAL(v3.UniverseCount(), 0);
  v3 = v2;
  BOOST_CHECK_EQUAL(v3.UniverseCount(), 2);
  BOOST_CHECK_EQUAL(v3.GetValue(DmxChannel(5, 0)), 255);
  BOOST_CHECK_EQUAL(v3.GetValue(DmxChannel(6, 0)), 42);
  v3 = v2; // this triggers optimization, hence the second test
  BOOST_CHECK_EQUAL(v3.UniverseCount(), 2);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace glight

