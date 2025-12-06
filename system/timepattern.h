#ifndef GLIGHT_SYSTEM_TIME_PATTERN_H_
#define GLIGHT_SYSTEM_TIME_PATTERN_H_

#include <chrono>
#include <cstdint>
#include <string>

#include "optionalnumber.h"

namespace glight::system {

class TimePattern {
 private:
  static constexpr bool IsDigit(char c) { return c >= '0' && c <= '9'; }

 public:
  OptionalNumber<uint16_t> year;
  OptionalNumber<uint16_t> month;
  OptionalNumber<uint16_t> day_of_month;
  OptionalNumber<uint16_t> day_of_week;

  OptionalNumber<uint16_t> hours;
  OptionalNumber<uint16_t> minutes;
  OptionalNumber<uint16_t> seconds;

  constexpr TimePattern() = default;

  TimePattern(const std::string& date_time_str) {
    SetDate(date_time_str);
    SetTime(date_time_str.substr(11));
  }

  TimePattern(const std::string& date_str, const std::string& time_str) {
    SetDate(date_str);
    SetTime(time_str);
  }

  void SetDate(const std::string& date_str) {
    day_of_week.Reset();
    if (date_str.size() >= 4 && IsDigit(date_str[0]) && IsDigit(date_str[1]) &&
        IsDigit(date_str[2]) && IsDigit(date_str[3])) {
      year = 1000 * (date_str[0] - '0') + 100 * (date_str[1] - '0') +
             10 * (date_str[2] - '0') + (date_str[3] - '0');
    } else {
      year.Reset();
    }
    if (date_str.size() >= 7 && IsDigit(date_str[5]) && IsDigit(date_str[6])) {
      month = 10 * (date_str[5] - '0') + (date_str[6] - '0');
      if (month < 1 || month > 12) month.Reset();
    } else {
      month.Reset();
    }
    if (date_str.size() >= 10 && IsDigit(date_str[8]) && IsDigit(date_str[9])) {
      day_of_month = 10 * (date_str[8] - '0') + (date_str[9] - '0');
      if (day_of_month < 1 || day_of_month > 31) day_of_month.Reset();
    } else {
      day_of_month.Reset();
    }
  }

  void SetTime(const std::string& time_str) {
    if (time_str.size() >= 2 && IsDigit(time_str[0]) && IsDigit(time_str[1])) {
      hours = 10 * (time_str[0] - '0') + (time_str[1] - '0');
      if (hours > 24) hours.Reset();
    } else {
      hours.Reset();
    }
    if (time_str.size() >= 5 && IsDigit(time_str[3]) && IsDigit(time_str[4])) {
      minutes = 10 * (time_str[3] - '0') + (time_str[4] - '0');
      if (minutes > 60) minutes.Reset();
    } else {
      minutes.Reset();
    }
    if (time_str.size() >= 8 && IsDigit(time_str[6]) && IsDigit(time_str[7])) {
      seconds = 10 * (time_str[6] - '0') + (time_str[7] - '0');
      if (seconds > 60) seconds.Reset();
    }
  }

  constexpr bool operator<=(const tm& compared_time) const {
    if (year) {
      if (*year > compared_time.tm_year + 1900)
        return false;
      else if (*year < compared_time.tm_year + 1900)
        return true;
    }
    if (month) {
      if (*month > compared_time.tm_mon + 1)
        return false;
      else if (*month < compared_time.tm_mon + 1)
        return true;
    }
    if (day_of_month) {
      if (*day_of_month > compared_time.tm_mday)
        return false;
      else if (*day_of_month < compared_time.tm_mday)
        return true;
    }
    if (day_of_week) {
      if (*day_of_week > compared_time.tm_wday)
        return true;
      else if (*day_of_week < compared_time.tm_wday)
        return false;
    }
    if (hours) {
      if (*hours > compared_time.tm_hour)
        return false;
      else if (*hours < compared_time.tm_hour)
        return true;
    }
    if (minutes) {
      if (*minutes > compared_time.tm_min) return false;
      if (*minutes < compared_time.tm_min) return true;
    }
    if (seconds) {
      if (*seconds > compared_time.tm_sec)
        return false;
      else if (*seconds < compared_time.tm_sec)
        return true;
    }
    return true;
  }

  constexpr bool operator>(const tm& compared_time) const {
    if (year) {
      if (*year < compared_time.tm_year + 1900)
        return false;
      else if (*year > compared_time.tm_year + 1900)
        return true;
    }
    if (month) {
      if (*month < compared_time.tm_mon + 1)
        return false;
      else if (*month > compared_time.tm_mon + 1)
        return true;
    }
    if (day_of_month) {
      if (*day_of_month < compared_time.tm_mday)
        return false;
      else if (*day_of_month > compared_time.tm_mday)
        return true;
    }
    if (day_of_week) {
      if (*day_of_week < compared_time.tm_wday)
        return false;
      else if (*day_of_week > compared_time.tm_wday)
        return true;
    }
    if (hours) {
      if (*hours < compared_time.tm_hour)
        return false;
      else if (*hours > compared_time.tm_hour)
        return true;
    }
    if (minutes) {
      if (*minutes < compared_time.tm_min) return false;
      if (*minutes > compared_time.tm_min) return true;
    }
    if (seconds) {
      if (*seconds < compared_time.tm_sec)
        return false;
      else if (*seconds > compared_time.tm_sec)
        return true;
    }
    return false;
  }
};

constexpr inline bool InRange(const TimePattern& start, const TimePattern& end,
                              const tm& value) {
  TimePattern e = end;
  if (!e.year) e.year = start.year;
  if (!e.month) e.month = start.month;
  if (!e.day_of_month) e.day_of_month = start.day_of_month;
  if (!e.day_of_week) e.day_of_week = start.day_of_week;
  if (!e.hours) e.hours = start.hours;
  if (!e.minutes) e.minutes = start.minutes;
  if (!e.seconds) e.seconds = start.seconds;
  return start <= value && end > value;
}

inline bool NowInRange(const TimePattern& start, const TimePattern& end) {
  const auto n = std::chrono::system_clock::now();
  const time_t tt = std::chrono::system_clock::to_time_t(n);
  const tm local_time = *localtime(&tt);
  return InRange(start, end, local_time);
}

inline std::string ToDateString(const TimePattern& pattern) {
  std::string result("XXXX-XX-XX");
  if (pattern.year) {
    const uint64_t year = *pattern.year;
    result[0] = year / 1000 + '0';
    result[1] = ((year / 100) % 10) + '0';
    result[2] = ((year / 10) % 10) + '0';
    result[3] = (year % 10) + '0';
  }
  if (pattern.month) {
    const uint64_t month = *pattern.month;
    result[5] = month / 10 + '0';
    result[6] = (month % 10) + '0';
  }
  if (pattern.day_of_month) {
    const uint64_t day = *pattern.day_of_month;
    result[8] = day / 10 + '0';
    result[9] = (day % 10) + '0';
  }
  return result;
}

inline std::string ToTimeString(const TimePattern& pattern) {
  std::string result("XX:XX:XX");
  if (pattern.hours) {
    const uint64_t hours = *pattern.hours;
    result[0] = hours / 10 + '0';
    result[1] = (hours % 10) + '0';
  }
  if (pattern.minutes) {
    const uint64_t minutes = *pattern.minutes;
    result[3] = minutes / 10 + '0';
    result[4] = (minutes % 10) + '0';
  }
  if (pattern.seconds) {
    const uint64_t seconds = *pattern.seconds;
    result[6] = seconds / 10 + '0';
    result[7] = (seconds % 10) + '0';
  }
  return result;
}

inline std::string ToString(const TimePattern& pattern) {
  std::string result("XXXX-XX-XX XX:XX:XX");
  if (pattern.year) {
    const uint64_t year = *pattern.year;
    result[0] = year / 1000 + '0';
    result[1] = ((year / 100) % 10) + '0';
    result[2] = ((year / 10) % 10) + '0';
    result[3] = (year % 10) + '0';
  }
  if (pattern.month) {
    const uint64_t month = *pattern.month;
    result[5] = month / 10 + '0';
    result[6] = (month % 10) + '0';
  }
  if (pattern.day_of_month) {
    const uint64_t day = *pattern.day_of_month;
    result[8] = day / 10 + '0';
    result[9] = (day % 10) + '0';
  }
  if (pattern.hours) {
    const uint64_t hours = *pattern.hours;
    result[11] = hours / 10 + '0';
    result[12] = (hours % 10) + '0';
  }
  if (pattern.minutes) {
    const uint64_t minutes = *pattern.minutes;
    result[14] = minutes / 10 + '0';
    result[15] = (minutes % 10) + '0';
  }
  if (pattern.seconds) {
    const uint64_t seconds = *pattern.seconds;
    result[17] = seconds / 10 + '0';
    result[18] = (seconds % 10) + '0';
  }
  return result;
}

/**
 * Returns a time pattern that will match the start of the current hour
 * on any day.
 */
inline TimePattern CurrentHourPattern() {
  TimePattern result;
  const auto n = std::chrono::system_clock::now();
  const time_t tt = std::chrono::system_clock::to_time_t(n);
  const tm local_time = *localtime(&tt);
  result.hours = local_time.tm_hour;
  result.minutes = 0;
  result.seconds = 0;
  return result;
}

}  // namespace glight::system

#endif
