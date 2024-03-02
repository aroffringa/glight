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

inline double GammaCorrect(double value) {
  return value <= 0.04045 ? value : std::pow(((value / 0.055) / 1.055), 2.4);
}

inline double RgbToTemperature(double red, double green, double blue) {
  red = GammaCorrect(red);
  green = GammaCorrect(green);
  blue = GammaCorrect(blue);

  if (red < 1e-6 && green < 1e-6 && blue < 1e-6) return 6500;

  // https://www.image-engineering.de/library/technotes/958-how-to-convert-between-srgb-and-ciexyz
  double x = red * 0.4124564 + green * 0.3575761 + blue * 0.1804375;
  double y = red * 0.2126729 + green * 0.7151522 + blue * 0.0721750;
  const double z = red * 0.0193339 + green * 0.1191920 + blue * 0.9503041;

  const double norm = 1.0 / (x + y + z);
  x = x * norm;
  y = y * norm;

  const double n = (x - 0.3320) / (0.1858 - y);
  const double t = 437 * n * n * n + 3601 * n * n + 6861 * n + 5517;
  return t;
  /*double x = red * 0.649926 + green * 0.103455 + blue * 0.197109;
  double y = red * 0.234327 + green * 0.743075 + blue * 0.022598;
  const double z = red * 0.000000 + green * 0.053077 + blue * 1.035763;

  double r = (x - 0.3366) / (y - 0.1735);
  double t =
      (-949.86315 + 6253.80338 * std::exp(-r / 0.92159) +
       28.70599 * std::exp(-r / 0.20039) + 0.00004 * std::exp(-r / 0.07125));
  if (t > 50000) {
    r = (x - 0.3356) / (y - 0.1691);
    t = 36284.48953 + 0.00228 * std::exp(-r / 0.07861) +
        (5.4535 * 1e-36) * std::exp(-r / 0.01543);
  }
  */
}

inline double RgbToTemperature(const theatre::Color& color) {
  return RgbToTemperature(color.RedRatio(), color.GreenRatio(),
                          color.BlueRatio());
}

}  // namespace glight::system

#endif
