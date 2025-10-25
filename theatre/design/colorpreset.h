#ifndef THEATRE_DESIGN_COLOR_PRESET_H_
#define THEATRE_DESIGN_COLOR_PRESET_H_

#include <vector>

#include "theatre/color.h"

#include "system/trackableptr.h"

namespace glight::theatre {

class Controllable;
class Color;
struct ColorDeduction;
struct DesignInfo;
class Folder;
class Management;
class PresetCollection;
class VariableEffect;

/**
 * Add a single preset value to a PresetCollection constructed
 * from a given color.
 */
void AddPresetValue(Management &management, Controllable &control,
                    PresetCollection &pc, const Color &color,
                    const ColorDeduction &deduction);

/**
 * Add a single preset value to a PresetCollection for which the color
 * can be controlled by a variable. A RgbMasterEffect is added in
 * between the preset collection and the variable to let the preset
 * collection control the intensity.
 */
void AddPresetValue(Management &management, Controllable &control,
                    PresetCollection &pc, VariableEffect *variable,
                    const ColorDeduction &deduction);

inline void AddPresetValue(Management &management, Controllable &control,
                           PresetCollection &pc,
                           const ColorOrVariable &color_or_variable,
                           const ColorDeduction &deduction) {
  std::visit(
      [&](auto &&arg) {
        AddPresetValue(management, control, pc, arg, deduction);
      },
      color_or_variable);
}

/**
 * Construct a single PresetCollection where controllables are given a color.
 * Each controllable in the list is assigned the color
 * with the same index from the list of colors.
 */
PresetCollection &MakeColorPreset(const DesignInfo &design,
                                  const std::vector<ColorOrVariable> &colors);

/**
 * Construct PresetCollections from a list of colors.
 * Each controllable in the list is assigned the color
 * with the same index from the list of colors, and a
 * separate PresetCollection is made for each.
 */
void MakeColorPresetPerFixture(const DesignInfo &design,
                               const std::vector<ColorOrVariable> &colors);

}  // namespace glight::theatre

#endif
