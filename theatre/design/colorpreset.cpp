#include "colorpreset.h"

#include "../colordeduction.h"
#include "../folder.h"
#include "../folderobject.h"
#include "../management.h"
#include "../presetcollection.h"

namespace glight::theatre {

void AddPresetValue(Management &management, Controllable &control,
                    PresetCollection &pc, const Color &color,
                    const ColorDeduction &deduction) {
  const unsigned red = color.Red() * ((1 << 24) - 1) / 255;
  const unsigned green = color.Green() * ((1 << 24) - 1) / 255;
  const unsigned blue = color.Blue() * ((1 << 24) - 1) / 255;
  const bool is_zero = (red == 0 && green == 0 && blue == 0);
  const unsigned master = is_zero ? 0 : (1 << 24) - 1;

  for (size_t i = 0; i != control.NInputs(); ++i) {
    const std::vector<Color> colors = control.InputColors(i);
    Color c = Color::Black();
    if (!colors.empty()) c = colors[0];
    SourceValue *sourceValue = management.GetSourceValue(control, i);
    if (control.InputType(i) == FunctionType::Master && master != 0) {
      pc.AddPresetValue(sourceValue->GetControllable(),
                        sourceValue->InputIndex())
          .SetValue(ControlValue(master));
    } else if (c == Color::White()) {
      if (deduction.whiteFromRGB) {
        const unsigned white = std::min(red, std::min(green, blue));
        if (white != 0) {
          pc.AddPresetValue(sourceValue->GetControllable(),
                            sourceValue->InputIndex())
              .SetValue(ControlValue(white));
        }
      }
    } else if (c == Color::Amber()) {
      if (deduction.amberFromRGB) {
        const unsigned amber = std::min(red / 2, green) * 2;
        if (amber != 0) {
          pc.AddPresetValue(sourceValue->GetControllable(),
                            sourceValue->InputIndex())
              .SetValue(ControlValue(amber));
        }
      }
    } else if (c == Color::UV()) {
      if (deduction.uvFromRGB) {
        const unsigned uv = std::min(blue / 3, red) * 3;
        if (uv != 0) {
          pc.AddPresetValue(sourceValue->GetControllable(),
                            sourceValue->InputIndex())
              .SetValue(ControlValue(uv));
        }
      }
    } else if (c == Color::Lime()) {
      if (deduction.limeFromRGB) {
        const unsigned lime = std::min(green / 2, red) * 2;
        if (lime != 0) {
          pc.AddPresetValue(sourceValue->GetControllable(),
                            sourceValue->InputIndex())
              .SetValue(ControlValue(lime));
        }
      }
    } else {
      if (c.Red() != 0 && red != 0)
        pc.AddPresetValue(sourceValue->GetControllable(),
                          sourceValue->InputIndex())
            .SetValue(ControlValue(red));
      if (c.Green() != 0 && green != 0)
        pc.AddPresetValue(sourceValue->GetControllable(),
                          sourceValue->InputIndex())
            .SetValue(ControlValue(green));
      if (c.Blue() != 0 && blue != 0)
        pc.AddPresetValue(sourceValue->GetControllable(),
                          sourceValue->InputIndex())
            .SetValue(ControlValue(blue));
    }
  }
}

PresetCollection &MakeColorPreset(
    Management &management, Folder &destination,
    const std::vector<Controllable *> &controllables,
    const std::vector<Color> &colors, const ColorDeduction &deduction) {
  PresetCollection &pc = management.AddPresetCollection();
  pc.SetName(destination.GetAvailableName("Colourpreset"));
  destination.Add(pc);
  for (size_t cIndex = 0; cIndex != controllables.size(); ++cIndex) {
    size_t colorIndex = cIndex % colors.size();
    AddPresetValue(management, *controllables[cIndex], pc, colors[colorIndex],
                   deduction);
  }
  management.AddSourceValue(pc, 0);
  return pc;
}

void MakeColorPresetPerFixture(Management &management, Folder &destination,
                               const std::vector<Controllable *> &controllables,
                               const std::vector<Color> &colors,
                               const ColorDeduction &deduction) {
  for (size_t cIndex = 0; cIndex != controllables.size(); ++cIndex) {
    PresetCollection &pc = management.AddPresetCollection();
    pc.SetName(destination.GetAvailableName("Colourpreset"));
    destination.Add(pc);
    size_t colorIndex = cIndex % colors.size();
    AddPresetValue(management, *controllables[cIndex], pc, colors[colorIndex],
                   deduction);
    management.AddSourceValue(pc, 0);
  }
}

}  // namespace glight::theatre
