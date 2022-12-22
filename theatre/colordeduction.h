#ifndef THEATRE_COLOR_DEDUCTION_H_
#define THEATRE_COLOR_DEDUCTION_H_

#include "controlvalue.h"

namespace glight::theatre {

inline ControlValue DeduceWhite(ControlValue r, ControlValue g, ControlValue b) {
  return Min(r, g, b);
}

inline ControlValue DeduceAmber(ControlValue r, ControlValue g, [[maybe_unused]] ControlValue b) {
  const unsigned amber = std::min(r.UInt() / 2, g.UInt()) * 2;
  return ControlValue(amber);
}

inline ControlValue DeduceUv(ControlValue r, [[maybe_unused]] ControlValue g, ControlValue b) {
  const unsigned uv = std::min(b.UInt() / 3, r.UInt()) * 3;
  return ControlValue(uv);
}

inline ControlValue DeduceLime(ControlValue r, ControlValue g, [[maybe_unused]] ControlValue b) {
  const unsigned lime = std::min(g.UInt() / 2, r.UInt()) * 2;
  return ControlValue(lime);
}

inline ControlValue DeduceColdWhite(ControlValue r, ControlValue g, ControlValue b) {
          const unsigned rg = std::min(r.UInt(), g.UInt());
          const unsigned cw = std::min(rg * 64, b.UInt() * 57) / 64;
   return ControlValue(cw);
}

inline ControlValue DeduceWarmWhite(ControlValue r, ControlValue g, ControlValue b) {
  const unsigned gb = std::min(g.UInt(), b.UInt());
  const unsigned ww = std::min(gb * 64, r.UInt() * 57) / 64;
  return ControlValue(ww);
}

} // namespace

#endif
