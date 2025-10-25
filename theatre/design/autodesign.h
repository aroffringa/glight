#ifndef THEATRE_AUTO_DESIGN_H_
#define THEATRE_AUTO_DESIGN_H_

#include <string>
#include <vector>

#include "theatre/color.h"
#include "theatre/forwards.h"

#include "system/trackableptr.h"

namespace glight::theatre {

class Chase;
class Color;
class Controllable;
struct DesignInfo;
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
  static Chase &MakeRunningLight(const DesignInfo &design,
                                 const std::vector<ColorOrVariable> &colors,
                                 RunType runType);

  static Chase &MakeColorVariation(const DesignInfo &design,
                                   const std::vector<ColorOrVariable> &colors,
                                   double variation);

  static Chase &MakeColorShift(const DesignInfo &design,
                               const std::vector<ColorOrVariable> &colors,
                               ShiftType shiftType);

  static Controllable &MakeVUMeter(const DesignInfo &design,
                                   const std::vector<ColorOrVariable> &colors,
                                   VUMeterDirection direction);

  static Chase &MakeIncreasingChase(const DesignInfo &design,
                                    const std::vector<ColorOrVariable> &colors,
                                    IncreasingType incType);

  static Effect &MakeFire(const DesignInfo &design,
                          const std::vector<ColorOrVariable> &colors);
};

}  // namespace glight::theatre

#endif
