#include "system/timepattern.h"

#include <boost/test/unit_test.hpp>

namespace glight::system {

BOOST_AUTO_TEST_SUITE(time_pattern)

BOOST_AUTO_TEST_CASE(date_string_conversion) {
  TimePattern pattern;
  BOOST_CHECK_EQUAL(ToDateString(pattern), "XXXX-XX-XX");
  pattern.year = 1982;
  BOOST_CHECK_EQUAL(ToDateString(pattern), "1982-XX-XX");
  pattern.month = 5;
  BOOST_CHECK_EQUAL(ToDateString(pattern), "1982-05-XX");
  pattern.day_of_month = 8;
  BOOST_CHECK_EQUAL(ToDateString(pattern), "1982-05-08");
}

BOOST_AUTO_TEST_CASE(time_string_conversion) {
  TimePattern pattern;
  BOOST_CHECK_EQUAL(ToTimeString(pattern), "XX:XX:XX");
  pattern.hours = 1;
  BOOST_CHECK_EQUAL(ToTimeString(pattern), "01:XX:XX");
  pattern.minutes = 2;
  BOOST_CHECK_EQUAL(ToTimeString(pattern), "01:02:XX");
  pattern.seconds = 3;
  BOOST_CHECK_EQUAL(ToTimeString(pattern), "01:02:03");
}

BOOST_AUTO_TEST_CASE(full_string_conversion) {
  TimePattern pattern;
  pattern.year = 1982;
  pattern.month = 5;
  pattern.day_of_month = 8;
  pattern.hours = 1;
  pattern.minutes = 2;
  pattern.seconds = 3;
  BOOST_CHECK_EQUAL(ToString(pattern), "1982-05-08 01:02:03");
}

BOOST_AUTO_TEST_CASE(set_from_string) {
  TimePattern pattern_a("XXXX-XX-XX", "XX:XX:XX");
  BOOST_CHECK_EQUAL(ToDateString(pattern_a), "XXXX-XX-XX");
  BOOST_CHECK_EQUAL(ToTimeString(pattern_a), "XX:XX:XX");

  TimePattern pattern_b("2082-08-05", "12:34:56");
  BOOST_CHECK_EQUAL(ToDateString(pattern_b), "2082-08-05");
  BOOST_CHECK_EQUAL(ToTimeString(pattern_b), "12:34:56");

  TimePattern pattern_c("2025-01-01 00:02:00");
  BOOST_CHECK_EQUAL(ToDateString(pattern_c), "2025-01-01");
  BOOST_CHECK_EQUAL(ToTimeString(pattern_c), "00:02:00");
}

BOOST_AUTO_TEST_CASE(compare) {
  tm compared_time;
  compared_time.tm_year = 2020 - 1900;
  compared_time.tm_mon = 9 - 1;
  compared_time.tm_mday = 18;
  compared_time.tm_hour = 14;
  compared_time.tm_min = 30;
  compared_time.tm_sec = 12;
  const TimePattern pattern_a("2020-09-18", "14:30:12");
  const TimePattern pattern_b("XXXX-XX-XX", "14:30:12");
  BOOST_CHECK(pattern_a <= compared_time);
  BOOST_CHECK(!(pattern_a > compared_time));
  BOOST_CHECK(pattern_b <= compared_time);
  BOOST_CHECK(!(pattern_b > compared_time));
  compared_time.tm_sec = 11;
  BOOST_CHECK(!(pattern_a <= compared_time));
  BOOST_CHECK(pattern_a > compared_time);
  BOOST_CHECK(!(pattern_b <= compared_time));
  BOOST_CHECK(pattern_b > compared_time);
  compared_time.tm_min = 31;
  BOOST_CHECK(pattern_a <= compared_time);
  BOOST_CHECK(!(pattern_a > compared_time));
  BOOST_CHECK(pattern_b <= compared_time);
  BOOST_CHECK(!(pattern_b > compared_time));
}

BOOST_AUTO_TEST_CASE(in_range) {
  tm compared_time;
  compared_time.tm_year = 2020 - 1900;
  compared_time.tm_mon = 9 - 1;
  compared_time.tm_mday = 18;
  compared_time.tm_hour = 14;
  compared_time.tm_min = 30;
  compared_time.tm_sec = 12;
  const TimePattern before_a("2019-09-19", "15:31:13");
  const TimePattern before_b("XXXX-XX-XX", "13:30:12");
  const TimePattern after_a("2020-09-19", "14:30:12");
  const TimePattern after_b("XXXX-XX-XX", "14:30:13");
  BOOST_CHECK(InRange(before_a, after_a, compared_time));
  BOOST_CHECK(InRange(before_a, after_b, compared_time));
  BOOST_CHECK(InRange(before_b, after_a, compared_time));
  BOOST_CHECK(InRange(before_b, after_b, compared_time));

  const TimePattern next_day("XXXX-XX-XX", "12:30:12");
  // TODO ideally this should work, because the set part of 'next_day' is before
  // 'before_a', and thus the range should go to the next day.
  // BOOST_CHECK(InRange(before_a, next_day, compared_time));
}

BOOST_AUTO_TEST_CASE(current) {
  const TimePattern hour = CurrentHourPattern();
  BOOST_CHECK(!hour.day_of_week);
  BOOST_CHECK(!hour.day_of_month);
  BOOST_CHECK(!hour.month);
  BOOST_CHECK(!hour.year);
  BOOST_REQUIRE(hour.hours);
  BOOST_REQUIRE(hour.minutes);
  BOOST_REQUIRE(hour.seconds);
  BOOST_CHECK_EQUAL(*hour.minutes, 0);
  BOOST_CHECK_EQUAL(*hour.seconds, 0);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace glight::system
