#ifndef THEATRE_DESIGN_ROTATION_H_
#define THEATRE_DESIGN_ROTATION_H_

#include <vector>

#include "theatre/color.h"

#include "system/trackableptr.h"

namespace glight::theatre {

struct DesignInfo;
class TimeSequence;

enum class RotationType { Forward, Backward, ForwardBackward };

TimeSequence &MakeRotation(const DesignInfo &design,
                           const std::vector<ColorOrVariable> &colors,
                           RotationType type);
}  // namespace glight::theatre

#endif
