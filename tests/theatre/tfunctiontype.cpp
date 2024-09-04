#include "theatre/functiontype.h"

#include <boost/test/unit_test.hpp>

#include <vector>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(function_type)

BOOST_AUTO_TEST_CASE(string_conversion) {
  const std::vector<FunctionType> function_types = GetFunctionTypes();
  BOOST_CHECK_GT(function_types.size(), 25);

  for (FunctionType t : function_types) {
    const std::string type_string = ToString(t);
    const FunctionType converted = GetFunctionType(type_string);
    BOOST_CHECK(t == converted);
  }
}

BOOST_AUTO_TEST_SUITE_END()
