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
  const unsigned char red =
      (temperature <= 6500)
          ? 255
          : std::clamp<int>(
                329.698727446 *
                    std::pow(0.01 * (temperature - 5900), -0.1332047592),
                0, 255);

  const unsigned char green =
      (temperature <= 6500)
          ? std::clamp<int>(
                99.4708025861 * std::log((temperature + 100) * 0.01) -
                    161.1195681661,
                0, 255)
          : std::clamp<int>(
                288.1221695283 *
                    std::pow((temperature - 5900) * 0.01, -0.0755148492),
                0, 255);

  unsigned char blue =
      (temperature >= 6500)
          ? 255
          : ((temperature <= 1900)
                 ? 0
                 : std::clamp<int>(
                       138.5177312231 * std::log(0.01 * (temperature - 900)) -
                           305.0447927307,
                       0, 255));

  return theatre::Color(red, green, blue);
}

}  // namespace glight::system

#endif
