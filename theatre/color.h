#ifndef THEATRE_COLOR_H_
#define THEATRE_COLOR_H_

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <variant>
#include <vector>

namespace glight::theatre {

class VariableEffect;

class Color {
 public:
  constexpr Color() = default;
  constexpr Color(unsigned char red, unsigned char green,
                  unsigned char blue) noexcept
      : red_(red), green_(green), blue_(blue) {}
  constexpr Color(const Color &) noexcept = default;
  Color &operator=(const Color &) noexcept = default;

  static constexpr Color FromRatio(double r, double g, double b) {
    return theatre::Color(unsigned(r * 255.4), unsigned(g * 255.4),
                          unsigned(b * 255.4));
  }
  static constexpr Color FromHexString(const char *s) {
    const auto from_hex = [](char c) -> int {
      if (c >= '1' && c <= '9')
        return c - '0';
      else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
      else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
      else
        return 0;
    };
    const unsigned char red = from_hex(s[0]) * 16 + from_hex(s[1]);
    const unsigned char blue = from_hex(s[2]) * 16 + from_hex(s[3]);
    const unsigned char green = from_hex(s[4]) * 16 + from_hex(s[5]);
    return Color(red, blue, green);
  }

  constexpr unsigned char Red() const { return red_; }
  constexpr unsigned char Green() const { return green_; }
  constexpr unsigned char Blue() const { return blue_; }

  constexpr double RedRatio() const { return red_ / 255.0; }
  constexpr double GreenRatio() const { return green_ / 255.0; }
  constexpr double BlueRatio() const { return blue_ / 255.0; }

  constexpr Color operator/(unsigned int divisor) const {
    return Color(red_ / divisor, green_ / divisor, blue_ / divisor);
  }

  /**
   * Calculates the maximum intensity color r, g, b values, as well
   * as the normalization factor to maximize the color. For example,
   * given the color {85, 0, 0} (dark red), the return color is
   * full red, as ratio, so {1, 0, 0}, and the normalization factor
   * is 1/3 (because 255 * 1 / 3 = 85).
   */
  constexpr std::array<double, 4> GetNormalizedRatios() const {
    double r = RedRatio();
    double g = GreenRatio();
    double b = BlueRatio();
    const double max_rgb = std::max({r, g, b});
    r /= max_rgb;
    g /= max_rgb;
    b /= max_rgb;
    return {r, g, b, max_rgb};
  }

  constexpr static Color Gray(unsigned char intensity) {
    return Color(intensity, intensity, intensity);
  }
  constexpr static Color Black() { return Color(0, 0, 0); }
  constexpr static Color White() { return Color(255, 255, 255); }
  constexpr static Color WhiteOrange() { return Color(255, 192, 128); }
  constexpr static Color RedC() { return Color(255, 0, 0); }
  constexpr static Color Amber() { return Orange(); }
  constexpr static Color Lime() { return GreenYellow(); }
  constexpr static Color Orange() { return Color(255, 128, 0); }
  constexpr static Color OrangeGreen() { return Color(170, 255, 0); }
  constexpr static Color GreenC() { return Color(0, 255, 0); }
  constexpr static Color GreenBlue() { return Color(0, 255, 255); }
  constexpr static Color GreenYellow() { return Color(128, 255, 0); }
  constexpr static Color BlueC() { return Color(0, 0, 255); }
  constexpr static Color LBlue() { return Color(0, 255, 255); }
  constexpr static Color Cyan() { return Color(0, 255, 255); }
  constexpr static Color BlueYellow() { return Color(192, 192, 255); }
  constexpr static Color Yellow() { return Color(255, 255, 0); }
  constexpr static Color YellowPurple() { return Color(255, 128, 255); }
  constexpr static Color Purple() { return Color(255, 0, 255); }
  constexpr static Color PurpleBlue() { return Color(128, 0, 255); }
  constexpr static Color PurpleWhite() { return Color(255, 128, 255); }
  constexpr static Color ColdWhite() { return White8500K(); }
  constexpr static Color WarmWhite() { return White3200K(); }
  constexpr static Color UV() { return Color(85, 0, 255); }

  constexpr static Color White2700K() { return Color(255, 180, 116); }
  constexpr static Color White3000K() { return Color(255, 180, 116); }
  constexpr static Color White3200K() { return Color(255, 186, 129); }
  constexpr static Color White4000K() { return Color(255, 208, 170); }
  constexpr static Color White4800K() { return Color(255, 226, 202); }
  constexpr static Color White6500K() { return White(); }
  constexpr static Color White7000K() { return Color(239, 240, 255); }
  constexpr static Color White8500K() { return Color(213, 225, 255); }
  constexpr static Color White10500K() { return Color(197, 215, 255); }

  static std::vector<Color> DefaultSet32();
  static std::vector<Color> DefaultSet40();

 private:
  unsigned char red_ = 0;
  unsigned char green_ = 0;
  unsigned char blue_ = 0;
};

inline Color operator*(const Color &lhs, unsigned char rhs) {
  return Color(lhs.Red() * rhs / 255, lhs.Green() * rhs / 255,
               lhs.Blue() * rhs / 255);
}

inline Color operator*(unsigned char lhs, const Color &rhs) {
  return Color(rhs.Red() * lhs / 255, rhs.Green() * lhs / 255,
               rhs.Blue() * lhs / 255);
}

inline bool operator==(const Color &lhs, const Color &rhs) {
  return lhs.Red() == rhs.Red() && lhs.Green() == rhs.Green() &&
         lhs.Blue() == rhs.Blue();
}

inline bool operator!=(const Color &lhs, const Color &rhs) {
  return lhs.Red() != rhs.Red() || lhs.Green() != rhs.Green() ||
         lhs.Blue() != rhs.Blue();
}

inline bool operator<(const Color &lhs, const Color &rhs) {
  if (lhs.Red() < rhs.Red())
    return true;
  else if (lhs.Red() > rhs.Red())
    return false;
  else if (lhs.Green() < rhs.Green())
    return true;
  else if (lhs.Green() > rhs.Green())
    return false;
  else
    return lhs.Blue() < rhs.Blue();
}

/**
 * @param h hue, value in the range 0 and 360.
 * @param s saturation, value in the 0-1.
 * @param l lightness, value in the range 0-1.
 */
inline void HslToRgb(double h, double s, double l, double &r, double &g,
                     double &b) {
  const double d = s * (1.0 - std::fabs(2.0 * l - 1.0));
  const double m = 255.0 * (l - 0.5 * d);
  const double x = d * (1.0 - std::fabs(std::fmod((h / 60.0), 2.0) - 1.0));
  if (0 <= h && h < 60.0) {
    r = 255.0 * d + m;
    g = 255.0 * x + m;
    b = m;
  } else if (60 <= h && h < 120) {
    r = 255.0 * x + m;
    g = 255.0 * d + m;
    b = m;
  } else if (120 <= h && h < 180.0) {
    r = m;
    g = 255.0 * d + m;
    b = 255.0 * x + m;
  } else if (180 <= h && h < 240.0) {
    r = m;
    g = 255.0 * x + m;
    b = 255.0 * d + m;
  } else if (240 <= h && h < 300.0) {
    r = 255.0 * x + m;
    g = m;
    b = 255.0 * d + m;
  } else if (300 <= h && h <= 360.0) {
    r = 255.0 * d + m;
    g = m;
    b = 255.0 * x + m;
  } else {
    r = 0.0;
    g = 0.0;
    b = 0.0;
  }
}

inline double ColorDistance(double r1, double g1, double b1, double r2,
                            double g2, double b2) {
  // from https://www.compuphase.com/cmetric.htm
  const double rmean = (r1 + r2) * 0.5;
  const double r = r1 - r2;
  const double g = g1 - g2;
  const double b = b1 - b2;
  return std::sqrt((((512.0 + rmean) * r * r) / 256.0) + 4.0 * g * g +
                   (((767.0 - rmean) * b * b) / 256.0));
}

inline std::string ToString(const Color &c) {
  return std::string("Red=") + std::to_string(int(c.Red())) +
         ", green=" + std::to_string(int(c.Green())) +
         ", blue=" + std::to_string(int(c.Blue()));
}

using ColorOrVariable = std::variant<Color, VariableEffect *>;

}  // namespace glight::theatre

#endif  // COLOR_H
