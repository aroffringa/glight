#ifndef THEATRE_AUTO_DESIGN_H_
#define THEATRE_AUTO_DESIGN_H_

#include <vector>

#include "theatre/color.h"
#include "theatre/forwards.h"

namespace glight::theatre {

class Chase;
class Color;
struct ColorDeduction;
class Controllable;
class Folder;
class Management;

enum class RunType {
  IncreasingRun,
  DecreasingRun,
  BackAndForthRun,
  InwardRun,
  OutwardRun,
  RandomRun
};

enum class ShiftType {
  IncreasingShift,
  DecreasingShift,
  BackAndForthShift,
  RandomShift
};

enum class VUMeterDirection { VUIncreasing, VUDecreasing, VUInward, VUOutward };

enum class IncreasingType {
  IncForward,
  IncBackward,
  IncForwardReturn,
  IncBackwardReturn
};

class AutoDesign {
 public:
  static Chase &MakeRunningLight(
      Management &management, Folder &destination,
      const std::vector<Controllable *> &controllables,
      const std::vector<ColorOrVariable> &colors,
      const ColorDeduction &deduction, RunType runType);

  static Chase &MakeColorVariation(
      Management &management, Folder &destination,
      const std::vector<Controllable *> &controllables,
      const std::vector<ColorOrVariable> &colors,
      const ColorDeduction &deduction, double variation);

  static Chase &MakeColorShift(Management &management, Folder &destination,
                               const std::vector<Controllable *> &controllables,
                               const std::vector<ColorOrVariable> &colors,
                               const ColorDeduction &deduction,
                               ShiftType shiftType);

  static Controllable &MakeVUMeter(
      Management &management, Folder &destination,
      const std::vector<Controllable *> &controllables,
      const std::vector<ColorOrVariable> &colors,
      const ColorDeduction &deduction, VUMeterDirection direction);

  static Chase &MakeIncreasingChase(
      Management &management, Folder &destination,
      const std::vector<Controllable *> &controllables,
      const std::vector<ColorOrVariable> &colors,
      const ColorDeduction &deduction, IncreasingType incType);

  static Effect &MakeFire(Management &management, Folder &destination,
                          const std::vector<Controllable *> &controllables,
                          const std::vector<ColorOrVariable> &colors,
                          const ColorDeduction &deduction);
};

}  // namespace glight::theatre

#endif
