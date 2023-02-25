#ifndef THEATRE_DESIGN_ROTATION_H_
#define THEATRE_DESIGN_ROTATION_H_

#include <vector>

namespace glight::theatre {

class Controllable;
class Color;
struct ColorDeduction;
class Folder;
class Management;
class TimeSequence;

enum class RotationType { Forward, Backward, ForwardBackward };

TimeSequence &MakeRotation(Management &management, Folder &destination,
                           const std::vector<Controllable *> &controllables,
                           const std::vector<Color> &colors,
                           const ColorDeduction &deduction, RotationType type);
}  // namespace glight::theatre

#endif
