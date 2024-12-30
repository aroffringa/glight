#ifndef THEATRE_DESIGN_ROTATION_H_
#define THEATRE_DESIGN_ROTATION_H_

#include <vector>

#include "theatre/color.h"

#include "system/trackableptr.h"

namespace glight::theatre {

class Controllable;
struct ColorDeduction;
class Folder;
class Management;
class TimeSequence;

enum class RotationType { Forward, Backward, ForwardBackward };

TimeSequence &MakeRotation(
    Management &management, Folder &destination,
    const std::vector<system::ObservingPtr<Controllable>> &controllables,
    const std::vector<ColorOrVariable> &colors, const ColorDeduction &deduction,
    RotationType type);
}  // namespace glight::theatre

#endif
