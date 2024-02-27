#ifndef THEATRE_POSITION_H_
#define THEATRE_POSITION_H_

#include <utility>

namespace glight::theatre {

class Position {
 public:
  constexpr Position() = default;

  constexpr Position(double x, double y) : x_(x), y_(y) {}

  constexpr double &X() { return x_; }
  constexpr double X() const { return x_; }

  constexpr double &Y() { return y_; }
  constexpr double Y() const { return y_; }

  constexpr bool operator==(const Position &rhs) const {
    return p() == rhs.p();
  }

  constexpr Position &operator+=(const Position &rhs) {
    x_ += rhs.x_;
    y_ += rhs.y_;
    return *this;
  }

  constexpr Position &operator+=(const std::pair<double, double> &rhs) {
    x_ += rhs.first;
    y_ += rhs.second;
    return *this;
  }

  constexpr bool operator<(const Position &rhs) const { return p() < rhs.p(); }

  constexpr Position operator*(double rhs) const {
    return Position(x_ * rhs, y_ * rhs);
  }

  constexpr Position operator/(double rhs) const {
    return Position(x_ / rhs, y_ / rhs);
  }

  constexpr Position operator+(Position &rhs) const {
    return Position(x_ + rhs.x_, y_ + rhs.y_);
  }

  constexpr Position operator+(const std::pair<double, double> &rhs) const {
    return Position(x_ + rhs.first, y_ + rhs.second);
  }

  constexpr Position operator-(const Position &rhs) const {
    return Position(x_ - rhs.x_, y_ - rhs.y_);
  }

  constexpr Position operator-(const std::pair<double, double> &rhs) const {
    return Position(x_ - rhs.first, y_ - rhs.second);
  }

  constexpr bool InsideRectangle(const Position &topleft,
                                 const Position &bottomright) const {
    return x_ >= topleft.x_ && y_ >= topleft.y_ && x_ < bottomright.x_ &&
           y_ < bottomright.y_;
  }

  constexpr Position Add(double x, double y) const {
    return Position(x_ + x, y_ + y);
  }

  constexpr double SquaredDistance(Position other) const {
    const double dx = x_ - other.x_, dy = y_ - other.y_;
    return dx * dx + dy * dy;
  }

 private:
  constexpr std::pair<double, double> p() const {
    return std::make_pair(x_, y_);
  }

  double x_ = 0.0;
  double y_ = 0.0;
};

}  // namespace glight::theatre

#endif
