#ifndef THEATRE_COLOR_H_
#define THEATRE_COLOR_H_

namespace glight::theatre {

class Color {
 public:
  constexpr Color(unsigned char red, unsigned char green, unsigned char blue)
      : red_(red), green_(green), blue_(blue) {}
  constexpr Color(const Color &) = default;
  Color &operator=(const Color &) = default;

  constexpr unsigned char Red() const { return red_; }
  constexpr unsigned char Green() const { return green_; }
  constexpr unsigned char Blue() const { return blue_; }

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

}  // namespace glight::theatre

#endif  // COLOR_H
