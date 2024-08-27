#ifndef GLIGHT_GUI_UNITS_H_
#define GLIGHT_GUI_UNITS_H_

#include <cmath>
#include <string>

namespace glight::gui {

inline std::string AngleToNiceString(double angle_in_rad) {
  const double degrees = angle_in_rad * 180.0 / M_PI;
  char str[20];
  snprintf(str, sizeof(str), "%.1f", degrees);
  return std::string(str);
}

}  // namespace glight::gui

#endif
