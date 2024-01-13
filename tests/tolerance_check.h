#ifndef THEATRE_TOLERANCE_CHECK_H_
#define THEATRE_TOLERANCE_CHECK_H_

#include <cmath>

#include <boost/test/unit_test.hpp>

#include "theatre/controlvalue.h"

namespace glight::theatre {

inline void ToleranceCheck(const ControlValue& value, unsigned expected,
                           unsigned tolerance = 1) {
  BOOST_CHECK_LE(
      std::abs(static_cast<int>(value.UInt()) - static_cast<int>(expected)),
      tolerance);
}

}  // namespace glight::theatre

#endif
