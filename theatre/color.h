#ifndef THEATRE_COLOR_H_
#define THEATRE_COLOR_H_

namespace glight::theatre {

class Color {
 public:
  constexpr Color(unsigned char red, unsigned char green, unsigned char blue)
      : red_(red), green_(green), blue_(blue) {}
  constexpr Color(const Color &) = default;
  Color &operator=(const Color &) = default;

  static Color FromHexString(const char *s) {
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
  constexpr static Color ColdWhite() { return Color(228, 228, 255); }
  constexpr static Color WarmWhite() { return Color(255, 228, 228); }
  constexpr static Color UV() { return Color(85, 0, 255); }

 private:
  unsigned char red_, green_, blue_;
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

}  // namespace glight::theatre

#endif  // COLOR_H
