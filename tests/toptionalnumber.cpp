#include "system/optionalnumber.h"

#include <cstdint>

namespace {

using glight::system::OptionalNumber;

template <typename T>
class Test {
  static_assert(!OptionalNumber<T>());
  static_assert(!OptionalNumber<T>().HasValue());
  static_assert(!OptionalNumber<T>(std::nullopt));
  static_assert(!OptionalNumber<T>{});
  static_assert(OptionalNumber<T>(T(0)));
  static_assert(OptionalNumber<T>(0).HasValue());
  static_assert(OptionalNumber<T>(T(13)).Value() == T(13));

  static constexpr OptionalNumber<T> GetAssigned() {
    OptionalNumber<T> number;
    return number = T(0.0);
  }

  static_assert(GetAssigned() == 0);
  static_assert(*GetAssigned() == 0);

  static constexpr OptionalNumber<T> GetReset() {
    OptionalNumber<T> number = GetAssigned();
    number.reset();
    return number;
  }

  static_assert(!GetReset());

  static constexpr OptionalNumber<T> GetUnassigned() {
    OptionalNumber<T> number(T(1));
    number = {};
    return number;
  }

  static_assert(!GetUnassigned());

  static_assert(OptionalNumber<T>().ValueOr(3) == 3);
  static_assert(OptionalNumber<T>(2).ValueOr(3) == 2);

  static_assert(OptionalNumber<T>() == OptionalNumber<T>());
  static_assert(OptionalNumber<T>(3) == OptionalNumber<T>(3));

  static_assert(OptionalNumber<T>() != OptionalNumber<T>(3));
  static_assert(OptionalNumber<T>(0) != OptionalNumber<T>(3));

  static_assert(OptionalNumber<T>() <= OptionalNumber<T>());
  static_assert(OptionalNumber<T>() <= OptionalNumber<T>(3));

  static_assert(OptionalNumber<T>() < OptionalNumber<T>(3));
  static_assert(OptionalNumber<T>(0) < OptionalNumber<T>(3));

  static_assert(OptionalNumber<T>() >= OptionalNumber<T>(3));
  static_assert(OptionalNumber<T>() >= OptionalNumber<T>());

  static_assert(OptionalNumber<T>(3) > OptionalNumber<T>());
  static_assert(OptionalNumber<T>(3) > OptionalNumber<T>(0));
};

template class Test<unsigned char>;
template class Test<int>;
template class Test<float>;
template class Test<double>;

static_assert(OptionalNumber<std::uint8_t>::UnsetValue == 255);
static_assert(OptionalNumber<std::int8_t>::UnsetValue == -128);
static_assert(OptionalNumber<std::uint16_t>::UnsetValue == 65535);
static_assert(OptionalNumber<std::int16_t>::UnsetValue == -32768);
static_assert(OptionalNumber<float>::UnsetValue ==
              std::numeric_limits<float>::min());
static_assert(OptionalNumber<double>::UnsetValue ==
              std::numeric_limits<double>::min());

}  // namespace
