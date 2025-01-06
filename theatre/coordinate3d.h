#ifndef THEATRE_COORDINATE_3D_H_
#define THEATRE_COORDINATE_3D_H_

#include "coordinate2d.h"

namespace glight::theatre {

class Coordinate3D {
 public:
  constexpr Coordinate3D() noexcept = default;
  constexpr Coordinate3D(double x, double y, double z) noexcept
      : x_(x), y_(y), z_(z) {}

  constexpr double &X() { return x_; }
  constexpr double X() const { return x_; }

  constexpr double &Y() { return y_; }
  constexpr double Y() const { return y_; }

  constexpr double &Z() { return z_; }
  constexpr double Z() const { return z_; }

  constexpr Coordinate2D XY() const { return Coordinate2D(x_, y_); }

 private:
  double x_ = 0.0;
  double y_ = 0.0;
  double z_ = 0.0;
};

}  // namespace glight::theatre

#endif
