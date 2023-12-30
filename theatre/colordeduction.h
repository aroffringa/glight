#ifndef THEATRE_COLOR_DEDUCTION_H_
#define THEATRE_COLOR_DEDUCTION_H_

#include "color.h"
#include "controlvalue.h"

namespace glight::theatre {

struct ColorDeduction {
  bool whiteFromRGB;
  bool amberFromRGB;
  bool uvFromRGB;
  bool limeFromRGB;
};

inline ControlValue DeduceWhite(ControlValue r, ControlValue g,
                                ControlValue b) {
  return Min(r, g, b);
}

inline ControlValue DeduceAmber(ControlValue r, ControlValue g,
                                [[maybe_unused]] ControlValue b) {
  const unsigned amber = std::min(r.UInt() / 2, g.UInt()) * 2;
  return ControlValue(amber);
}

inline ControlValue DeduceUv(ControlValue r, [[maybe_unused]] ControlValue g,
                             ControlValue b) {
  const unsigned uv = std::min(b.UInt() / 3, r.UInt()) * 3;
  return ControlValue(uv);
}

inline ControlValue DeduceLime(ControlValue r, ControlValue g,
                               [[maybe_unused]] ControlValue b) {
  const unsigned lime = std::min(g.UInt() / 2, r.UInt()) * 2;
  return ControlValue(lime);
}

inline ControlValue DeduceColdWhite(ControlValue r, ControlValue g,
                                    ControlValue b) {
  const unsigned rg = std::min(r.UInt(), g.UInt());
  const unsigned cw = std::min(rg * 64, b.UInt() * 57) / 64;
  return ControlValue(cw);
}

inline ControlValue DeduceWarmWhite(ControlValue r, ControlValue g,
                                    ControlValue b) {
  const unsigned gb = std::min(g.UInt(), b.UInt());
  const unsigned ww = std::min(gb * 64, r.UInt() * 57) / 64;
  return ControlValue(ww);
}

/**
 * Solves the equation f1 a + f2 b <= rgb_values, such that the 1-norm of the
 * difference is minimized.
 */
void Solve2ColorFit(Color a, Color b, const ControlValue* rgb_values,
                    ControlValue* fitted_values);

/**
 * Solves the equation f1 a + f2 b + f3 c <= rgb_values, such that the 1-norm of
 * the difference is minimized.
 */
void Solve3ColorFit(Color a, Color b, Color c, const ControlValue* rgb_values,
                    ControlValue* fitted_values);

void Normalized3ColorFit(Color a, Color b, Color c,
                         const ControlValue* rgb_values,
                         ControlValue* fitted_values);

}  // namespace glight::theatre

#endif
