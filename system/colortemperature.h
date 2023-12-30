#ifndef GLIGHT_SYSTEM_COLOR_TEMPERATURE_H_
#define GLIGHT_SYSTEM_COLOR_TEMPERATURE_H_

#include "../theatre/color.h"

#include <algorithm>
#include <cmath>

namespace glight::system {

/**
 * Convert a color temperature to RGB values.
 * @param temperature should be between 1100 and 40100.
 */
inline theatre::Color TemperatureToRgb(unsigned temperature) {
  unsigned char red;
  if (temperature > 6500) {
    const double term = std::pow(0.01 * (temperature - 5900), -0.1332047592);
    red = std::clamp<int>(329.698727446 * term, 0, 255);
  } else {
    red = 255;
  }

  unsigned char green;
  if (temperature <= 6500) {
    const double term = std::log((temperature + 100) * 0.01);
    green = std::clamp<int>(99.4708025861 * term - 161.1195681661, 0, 255);
  } else {
    const double term = std::pow((temperature - 5900) * 0.01, -0.0755148492);
    green = std::clamp<int>(288.1221695283 * term, 0, 255);
  }

  unsigned char blue;
  if (temperature < 6500) {
    if (temperature <= 1900) {
      blue = 0;
    } else {
      const double term = 138.5177312231 * std::log(0.01 * (temperature - 900));
      blue = std::clamp<int>(term - 305.0447927307, 0, 255);
    }
  } else {
    blue = 255;
  }

  return theatre::Color(red, green, blue);
}

}  // namespace glight::system

#endif
