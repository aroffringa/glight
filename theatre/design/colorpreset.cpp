#include "colorpreset.h"

#include "theatre/colordeduction.h"
#include "theatre/folder.h"
#include "theatre/folderobject.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"

#include "theatre/effects/rgbmastereffect.h"
#include "theatre/effects/variableeffect.h"

namespace glight::theatre {

using system::ObservingPtr;

void AddPresetValue(Management &management, Controllable &control,
                    PresetCollection &pc, const Color &color,
                    const ColorDeduction &deduction) {
  const ControlValue red = ControlValue::FromUChar(color.Red());
  const ControlValue green = ControlValue::FromUChar(color.Green());
  const ControlValue blue = ControlValue::FromUChar(color.Blue());
  const bool is_zero = !red && !green && !blue;
  const unsigned master = is_zero ? 0 : (1 << 24) - 1;

  for (size_t i = 0; i != control.NInputs(); ++i) {
    const FunctionType type = control.InputType(i);
    SourceValue *sourceValue = management.GetSourceValue(control, i);
    switch (type) {
      case FunctionType::Master:
        if (master != 0) {
          pc.AddPresetValue(control, sourceValue->InputIndex())
              .SetValue(ControlValue(master));
        }
        break;
      case FunctionType::Red:
        if (red) {
          pc.AddPresetValue(control, sourceValue->InputIndex()).SetValue(red);
        }
        break;
      case FunctionType::Green:
        if (green) {
          pc.AddPresetValue(control, sourceValue->InputIndex()).SetValue(green);
        }
        break;
      case FunctionType::Blue:
        if (blue) {
          pc.AddPresetValue(control, sourceValue->InputIndex()).SetValue(blue);
        }
        break;
      case FunctionType::White:
        if (deduction.whiteFromRGB) {
          const ControlValue white = DeduceWhite(red, green, blue);
          if (white) {
            pc.AddPresetValue(control, sourceValue->InputIndex())
                .SetValue(ControlValue(white));
          }
        }
        break;
      case FunctionType::WarmWhite:
        if (deduction.whiteFromRGB) {
          const ControlValue ww = DeduceWarmWhite(red, green, blue);
          if (ww) {
            pc.AddPresetValue(control, sourceValue->InputIndex())
                .SetValue(ControlValue(ww));
          }
        }
        break;
      case FunctionType::ColdWhite:
        if (deduction.whiteFromRGB) {
          const ControlValue cw = DeduceColdWhite(red, green, blue);
          if (cw) {
            pc.AddPresetValue(control, sourceValue->InputIndex())
                .SetValue(ControlValue(cw));
          }
        }
        break;
      case FunctionType::Amber:
        if (deduction.amberFromRGB) {
          const ControlValue amber = DeduceAmber(red, green, blue);
          if (amber) {
            pc.AddPresetValue(control, sourceValue->InputIndex())
                .SetValue(amber);
          }
        }
        break;
      case FunctionType::UV:
        if (deduction.uvFromRGB) {
          const ControlValue uv = DeduceUv(red, green, blue);
          if (uv) {
            pc.AddPresetValue(control, sourceValue->InputIndex()).SetValue(uv);
          }
        }
        break;
      case FunctionType::Lime:
        if (deduction.limeFromRGB) {
          const ControlValue lime = DeduceLime(red, green, blue);
          if (lime) {
            pc.AddPresetValue(control, sourceValue->InputIndex())
                .SetValue(lime);
          }
        }
        break;
      default:
        break;
    }
  }
}

void AddPresetValue(Management &management, Controllable &control,
                    PresetCollection &pc, VariableEffect *variable,
                    const ColorDeduction &deduction) {
  std::unique_ptr<RgbMasterEffect> effect = std::make_unique<RgbMasterEffect>();
  effect->SetName(pc.Parent().GetAvailableName(pc.Name() + "_var"));
  Effect &added_effect = static_cast<Effect &>(
      *management.AddEffect(std::move(effect), pc.Parent()));
  for (size_t inp = 0; inp != added_effect.NInputs(); ++inp)
    management.AddSourceValue(added_effect, inp);

  pc.AddPresetValue(added_effect, RgbMasterEffect::kMasterInput)
      .SetValue(ControlValue::Max());
  variable->AddConnection(added_effect, RgbMasterEffect::kRedInput);
  variable->AddConnection(added_effect, RgbMasterEffect::kGreenInput);
  variable->AddConnection(added_effect, RgbMasterEffect::kBlueInput);

  for (size_t i = 0; i != control.NInputs(); ++i) {
    const FunctionType type = control.InputType(i);
    if (type == FunctionType::Master) {
      pc.AddPresetValue(control, i).SetValue(ControlValue::Max());
    } else if (type == FunctionType::Red || type == FunctionType::Green ||
               type == FunctionType::Blue) {
      added_effect.AddConnection(control, i);
    }
  }
}

PresetCollection &MakeColorPreset(
    Management &management, Folder &destination,
    const std::vector<ObservingPtr<Controllable>> &controllables,
    const std::vector<ColorOrVariable> &colors,
    const ColorDeduction &deduction) {
  ObservingPtr<PresetCollection> pc = management.AddPresetCollectionPtr();
  pc->SetName(destination.GetAvailableName("Colourpreset"));
  destination.Add(pc);
  for (size_t cIndex = 0; cIndex != controllables.size(); ++cIndex) {
    size_t colorIndex = cIndex % colors.size();
    AddPresetValue(management, *controllables[cIndex], *pc, colors[colorIndex],
                   deduction);
  }
  management.AddSourceValue(*pc, 0);
  return *pc;
}

void MakeColorPresetPerFixture(
    Management &management, Folder &destination,
    const std::vector<ObservingPtr<Controllable>> &controllables,
    const std::vector<ColorOrVariable> &colors,
    const ColorDeduction &deduction) {
  for (size_t cIndex = 0; cIndex != controllables.size(); ++cIndex) {
    ObservingPtr<PresetCollection> pc_ptr = management.AddPresetCollectionPtr();
    PresetCollection &pc = *pc_ptr;
    pc.SetName(destination.GetAvailableName("Colourpreset"));
    destination.Add(std::move(pc_ptr));
    size_t colorIndex = cIndex % colors.size();
    AddPresetValue(management, *controllables[cIndex], pc, colors[colorIndex],
                   deduction);
    management.AddSourceValue(pc, 0);
  }
}

}  // namespace glight::theatre
