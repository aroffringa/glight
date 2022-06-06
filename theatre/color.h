#ifndef COLOR_H
#define COLOR_H

class Color {
 public:
  constexpr Color(unsigned char red, unsigned char green, unsigned char blue)
      : _red(red), _green(green), _blue(blue) {}
  constexpr Color(const Color &) = default;
  Color &operator=(const Color &) = default;

  constexpr unsigned char Red() const { return _red; }
  constexpr unsigned char Green() const { return _green; }
  constexpr unsigned char Blue() const { return _blue; }

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

  static Color H20Color(unsigned value) {
    if (value <= 10)
      return White();
    else if (value <= 21)
      return WhiteOrange();
    else if (value <= 32)
      return Orange();
    else if (value <= 43)
      return OrangeGreen();
    else if (value <= 54)
      return GreenC();
    else if (value <= 65)
      return GreenBlue();
    else if (value <= 76)
      return BlueC();
    else if (value <= 87)
      return BlueYellow();
    else if (value <= 98)
      return Yellow();
    else if (value <= 109)
      return YellowPurple();
    else if (value <= 120)
      return Purple();
    else if (value <= 127)
      return PurpleWhite();
    else
      return White();
  }

  static Color BTMacroColor(unsigned value) {
    if (value < 68) {
      if (value < 28) {
        if (value < 8)
          return Black();
        else
          return RedC();
      } else {
        if (value < 48)
          return Orange();
        else
          return Yellow();
      }
    } else {
      if (value < 108) {
        if (value < 88)
          return GreenC();
        else if (value < 98)
          return Cyan();
        else
          return BlueC();
      } else {
        if (value < 118)
          return Purple();
        else
          return White();  // color fade
      }
    }
  }

 private:
  unsigned char _red, _green, _blue;
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

#endif  // COLOR_H
