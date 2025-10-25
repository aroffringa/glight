#include "rotation.h"

#include <cassert>

#include "colorpreset.h"
#include "designinfo.h"

#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/timesequence.h"

namespace glight::theatre {

using system::ObservingPtr;

TimeSequence &MakeRotation(const DesignInfo &design,
                           const std::vector<ColorOrVariable> &colors,
                           RotationType type) {
  Management &management = *design.management;
  Folder &destination = *design.destination;

  assert(!design.controllables->empty());
  assert(!colors.empty());

  const ObservingPtr<TimeSequence> result_ptr = management.AddTimeSequencePtr();
  TimeSequence &result = *result_ptr;
  result.SetName(GetValidName(design, "Rotation"));
  result.SetRepeatCount(0);
  destination.Add(result_ptr);
  management.AddSourceValue(result, 0);

  std::vector<ColorOrVariable> modified_colors = colors;
  while (modified_colors.size() < design.controllables->size()) {
    modified_colors.emplace_back(Color::Black());
  }

  std::vector<ColorOrVariable> step_colors(design.controllables->size(),
                                           Color::Black());
  for (size_t offset = 0; offset != colors.size(); ++offset) {
    for (size_t i = 0; i != design.controllables->size(); ++i) {
      step_colors[i] = modified_colors[(i + offset) % modified_colors.size()];
    }
    const ColorOrVariable back = step_colors.back();
    step_colors.back() = Color::Black();
    DesignInfo pc_design(design);
    pc_design.name = "";
    PresetCollection &pc1 = MakeColorPreset(pc_design, step_colors);
    pc1.SetName(destination.GetAvailableName(result.Name() + "_"));
    TimeSequence::Step &step1 = result.AddStep(pc1, 0);
    step1.transition.SetType(TransitionType::Fade);
    step1.transition.SetLengthInMs(500);
    step1.trigger.SetDelayInMs(0);

    step_colors.back() = back;
    step_colors.front() = Color::Black();
    PresetCollection &pc2 = MakeColorPreset(pc_design, step_colors);
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
