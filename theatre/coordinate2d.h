#ifndef THEATRE_COORDINATE_2D_H_
#define THEATRE_COORDINATE_2D_H_

#include <cmath>
#include <utility>

namespace glight::theatre {

class Coordinate2D {
 public:
  constexpr Coordinate2D() = default;

  constexpr Coordinate2D(double x, double y) : x_(x), y_(y) {}

  constexpr double &X() { return x_; }
  constexpr double X() const { return x_; }

  constexpr double &Y() { return y_; }
  constexpr double Y() const { return y_; }

  constexpr bool operator==(const Coordinate2D &rhs) const {
    return p() == rhs.p();
  }

  constexpr Coordinate2D &operator+=(const Coordinate2D &rhs) {
    x_ += rhs.x_;
    y_ += rhs.y_;
    return *this;
  }

  constexpr Coordinate2D &operator+=(const std::pair<double, double> &rhs) {
    x_ += rhs.first;
    y_ += rhs.second;
    return *this;
  }

  constexpr bool operator<(const Coordinate2D &rhs) const {
    return p() < rhs.p();
  }

  constexpr Coordinate2D operator*(double rhs) const {
    return Coordinate2D(x_ * rhs, y_ * rhs);
  }

  constexpr Coordinate2D operator/(double rhs) const {
    return Coordinate2D(x_ / rhs, y_ / rhs);
  }

  constexpr Coordinate2D operator+(const Coordinate2D &rhs) const {
    return Coordinate2D(x_ + rhs.x_, y_ + rhs.y_);
  }

  constexpr Coordinate2D operator+(const std::pair<double, double> &rhs) const {
    return Coordinate2D(x_ + rhs.first, y_ + rhs.second);
  }

  constexpr Coordinate2D operator-(const Coordinate2D &rhs) const {
    return Coordinate2D(x_ - rhs.x_, y_ - rhs.y_);
  }

  constexpr Coordinate2D operator-(const std::pair<double, double> &rhs) const {
    return Coordinate2D(x_ - rhs.first, y_ - rhs.second);
  }

  constexpr bool InsideRectangle(const Coordinate2D &topleft,
                                 const Coordinate2D &bottomright) const {
    return x_ >= topleft.x_ && y_ >= topleft.y_ && x_ < bottomright.x_ &&
           y_ < bottomright.y_;
  }

  constexpr Coordinate2D Add(double x, double y) const {
    return Coordinate2D(x_ + x, y_ + y);
  }

  constexpr double SquaredDistance(Coordinate2D other) const {
    const double dx = x_ - other.x_, dy = y_ - other.y_;
    return dx * dx + dy * dy;
  }

  double Angle() const { return std::atan2(y_, x_); }

 private:
  constexpr std::pair<double, double> p() const {
    return std::make_pair(x_, y_);
  }

  double x_ = 0.0;
  double y_ = 0.0;
};

}  // namespace glight::theatre

#endif
