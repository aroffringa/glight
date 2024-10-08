#ifndef GLIGHT_GUI_UNITS_H_
#define GLIGHT_GUI_UNITS_H_

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

namespace glight::gui {

inline std::string AngleToNiceString(double angle_in_rad) {
  const double degrees = angle_in_rad * 180.0 / M_PI;
  char str[20];
  snprintf(str, sizeof(str), "%.1f", degrees);
  return std::string(str);
}

inline std::string MetersToString(double meters) {
  char str[20];
  snprintf(str, sizeof(str), "%.3f", meters);
  return std::string(str);
}

inline std::string ToRoundedString(double value, int decimals = 1) {
  std::ostringstream str;
  str << std::fixed << std::setprecision(decimals) << value << '\n';
  return str.str();
}

}  // namespace glight::gui

#endif
