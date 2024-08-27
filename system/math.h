#ifndef GLIGHT_SYSTEM_MATH_H_
#define GLIGHT_SYSTEM_MATH_H_

#include <cmath>

namespace glight::system {

// In C++20, RadialDistance and RadialClamp can become constexpr

template <typename T>
inline T RadialDistance(T angle_a, T angle_b) {
  double distance = std::fmod(angle_a - angle_b, 2.0 * M_PI);
  if (distance < -M_PI)
    distance += 2.0 * M_PI;
  else if (distance > M_PI)
    distance -= 2.0 * M_PI;
  return std::fabs(distance);
}

/**
 * Turn and clamp an angle between two values. It will add/subtract
 * as many values of 2pi until it is between @p low and @p high. If this
 * is not possible (e.g. when low=0 and high=1, and angle=2), then it
 * will set angle to either low or high, whichever is radially closer.
 */
template <typename T>
inline T RadialClamp(T angle, T low, T high) {
  const T low_rotations = std::floor(low / (2.0 * M_PI));
  const T angle_rotations = std::floor(angle / (2.0 * M_PI));
  // Make sure that angle has at least as many rotations as low
  angle = angle + (low_rotations - angle_rotations) * 2 * M_PI;
  if (angle < low) angle += 2.0 * M_PI;
  if (angle > high) angle -= 2.0 * M_PI;
  if (angle < low) {
    // Chose closest side
    return RadialDistance(angle, low) > RadialDistance(angle, high) ? high
                                                                    : low;
  }
  return angle;
}

}  // namespace glight::system

#endif
