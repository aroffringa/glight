#ifndef THEATRE_POSITION_H_
#define THEATRE_POSITION_H_

#include <utility>

namespace glight::theatre {

class Position {
 public:
  Position() = default;

  Position(double x, double y) : x_(x), y_(y) {}

  double &X() { return x_; }
  const double &X() const { return x_; }

  double &Y() { return y_; }
  const double &Y() const { return y_; }

  bool operator==(const Position &rhs) const { return p() == rhs.p(); }

  Position &operator+=(const std::pair<double, double> &rhs) {
    x_ += rhs.first;
    y_ += rhs.second;
    return *this;
  }

  bool operator<(const Position &rhs) const { return p() < rhs.p(); }

  Position operator*(double rhs) const { return Position(x_ * rhs, y_ * rhs); }

  Position operator/(double rhs) const { return Position(x_ / rhs, y_ / rhs); }

  Position operator+(const std::pair<double, double> &add) const {
    return Position(x_ + add.first, y_ + add.second);
  }

  std::pair<double, double> operator-(const Position &rhs) const {
    return std::make_pair(x_ - rhs.x_, y_ - rhs.y_);
  }

  Position operator-(const std::pair<double, double> &add) const {
    return Position(x_ - add.first, y_ - add.second);
  }

  bool InsideRectangle(const Position &topleft,
                       const Position &bottomright) const {
    return x_ >= topleft.x_ && y_ >= topleft.y_ && x_ < bottomright.x_ &&
           y_ < bottomright.y_;
  }

  Position Add(double x, double y) const { return Position(x_ + x, y_ + y); }

  double SquaredDistance(Position other) const {
    const double dx = x_ - other.x_, dy = y_ - other.y_;
    return dx * dx + dy * dy;
  }

 private:
  std::pair<double, double> p() const { return std::make_pair(x_, y_); }

  double x_ = 0.0;
  double y_ = 0.0;
};

}  // namespace glight::theatre

#endif
