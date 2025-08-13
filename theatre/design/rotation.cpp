#include "rotation.h"

#include <cassert>

#include "colorpreset.h"

#include "../folder.h"
#include "../management.h"
#include "../timesequence.h"

namespace glight::theatre {

using system::ObservingPtr;

TimeSequence &MakeRotation(
    Management &management, Folder &destination,
    const std::vector<system::ObservingPtr<Controllable>> &controllables,
    const std::vector<ColorOrVariable> &colors, const ColorDeduction &deduction,
    RotationType type) {
  assert(!controllables.empty());
  assert(!colors.empty());

  const ObservingPtr<TimeSequence> result_ptr =
      management.AddTimeSequencePtr();
  TimeSequence &result = *result_ptr;
  result.SetName(destination.GetAvailableName("Rotation"));
  result.SetRepeatCount(0);
  destination.Add(result_ptr);
  management.AddSourceValue(result, 0);

  std::vector<ColorOrVariable> modified_colors = colors;
  while (modified_colors.size() < controllables.size()) {
    modified_colors.emplace_back(Color::Black());
  }

  std::vector<ColorOrVariable> step_colors(controllables.size(),
                                           Color::Black());
  for (size_t offset = 0; offset != colors.size(); ++offset) {
    for (size_t i = 0; i != controllables.size(); ++i) {
      step_colors[i] = modified_colors[(i + offset) % modified_colors.size()];
    }
    const ColorOrVariable back = step_colors.back();
    step_colors.back() = Color::Black();
    PresetCollection &pc1 = MakeColorPreset(
        management, destination, controllables, step_colors, deduction);
    pc1.SetName(destination.GetAvailableName(result.Name() + "_"));
    TimeSequence::Step &step1 = result.AddStep(pc1, 0);
    step1.transition.SetType(TransitionType::Fade);
    step1.transition.SetLengthInMs(500);
    step1.trigger.SetDelayInMs(0);

    step_colors.back() = back;
    step_colors.front() = Color::Black();
    PresetCollection &pc2 = MakeColorPreset(
        management, destination, controllables, step_colors, deduction);
    TimeSequence::Step &step2 = result.AddStep(pc2, 0);
    pc2.SetName(destination.GetAvailableName(result.Name() + "_"));
    step2.transition.SetLengthInMs(0);
    step2.trigger.SetDelayInMs(0);
  }
  if (type == RotationType::ForwardBackward) {
  }
  return result;
}

}  // namespace glight::theatre
