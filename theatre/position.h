#ifndef THEATRE_POSITION_H_
#define THEATRE_POSITION_H_

#include <utility>

namespace glight::theatre {

class Position {
 public:
  Position() : _x(0.0), _y(0.0) {}

  Position(double x, double y) : _x(x), _y(y) {}

  double &X() { return _x; }
  const double &X() const { return _x; }

  double &Y() { return _y; }
  const double &Y() const { return _y; }

  bool operator==(const Position &rhs) const { return p() == rhs.p(); }

  Position &operator+=(const std::pair<double, double> &rhs) {
    _x += rhs.first;
    _y += rhs.second;
    return *this;
  }

  bool operator<(const Position &rhs) const { return p() < rhs.p(); }

  Position operator*(double rhs) const { return Position(_x * rhs, _y * rhs); }

  Position operator/(double rhs) const { return Position(_x / rhs, _y / rhs); }

  Position operator+(const std::pair<double, double> &add) const {
    return Position(_x + add.first, _y + add.second);
  }

  std::pair<double, double> operator-(const Position &rhs) const {
    return std::make_pair(_x - rhs._x, _y - rhs._y);
  }

  Position operator-(const std::pair<double, double> &add) const {
    return Position(_x - add.first, _y - add.second);
  }

  bool InsideRectangle(const Position &topleft,
                       const Position &bottomright) const {
    return _x >= topleft._x && _y >= topleft._y && _x < bottomright._x &&
           _y < bottomright._y;
  }

  Position Add(double x, double y) const { return Position(_x + x, _y + y); }

  double SquaredDistance(Position other) const {
    double dx = _x - other._x, dy = _y - other._y;
    return dx * dx + dy * dy;
  }

 private:
  std::pair<double, double> p() const { return std::make_pair(_x, _y); }

  double _x, _y;
};

}  // namespace glight::theatre

#endif
