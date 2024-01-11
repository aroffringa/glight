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

  static class Chase &MakeColorVariation(
      class Management &management, class Folder &destination,
      const std::vector<class Controllable *> &controllables,
      const std::vector<Color> &colors, const ColorDeduction &deduction,
      double variation);

  static class Chase &MakeColorShift(
      class Management &management, class Folder &destination,
      const std::vector<class Controllable *> &controllables,
      const std::vector<Color> &colors, const ColorDeduction &deduction,
      ShiftType shiftType);

  static class Controllable &MakeVUMeter(
      class Management &management, class Folder &destination,
      const std::vector<class Controllable *> &controllables,
      const std::vector<Color> &colors, const ColorDeduction &deduction,
      VUMeterDirection direction);

  static class Chase &MakeIncreasingChase(
      class Management &management, class Folder &destination,
      const std::vector<class Controllable *> &controllables,
      const std::vector<Color> &colors, const ColorDeduction &deduction,
      IncreasingType incType);
};

}  // namespace glight::theatre

#endif
