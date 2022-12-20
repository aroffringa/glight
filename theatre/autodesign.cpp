#include "autodesign.h"

#include "chase.h"
#include "color.h"
#include "controllable.h"
#include "folder.h"
#include "management.h"
#include "presetcollection.h"
#include "sequence.h"

#include "effects/audioleveleffect.h"
#include "effects/thresholdeffect.h"

#include <algorithm>
#include <random>

namespace glight::theatre {

void AutoDesign::addColorPresets(Management &management, Controllable &control,
                                 PresetCollection &pc, const Color &color,
                                 const ColorDeduction &deduction) {
  unsigned red = color.Red() * ((1 << 24) - 1) / 255;
  unsigned green = color.Green() * ((1 << 24) - 1) / 255;
  unsigned blue = color.Blue() * ((1 << 24) - 1) / 255;
  unsigned master = 0;
  if (red != 0 || green != 0 || blue != 0) master = (1 << 24) - 1;

  for (size_t i = 0; i != control.NInputs(); ++i) {
    std::vector<Color> colors = control.InputColors(i);
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

PresetCollection &AutoDesign::MakeColorPreset(
    class Management &management, class Folder &destination,
    const std::vector<class Controllable *> &controllables,
    const std::vector<class Color> &colors, const ColorDeduction &deduction) {
  PresetCollection &pc = management.AddPresetCollection();
  pc.SetName(destination.GetAvailableName("Colourpreset"));
  destination.Add(pc);
  for (size_t cIndex = 0; cIndex != controllables.size(); ++cIndex) {
    size_t colorIndex = cIndex % colors.size();
    addColorPresets(management, *controllables[cIndex], pc, colors[colorIndex],
                    deduction);
  }
  management.AddSourceValue(pc, 0);
  return pc;
}

void AutoDesign::MakeColorPresetPerFixture(
    class Management &management, class Folder &destination,
    const std::vector<class Controllable *> &controllables,
    const std::vector<class Color> &colors, const ColorDeduction &deduction) {
  for (size_t cIndex = 0; cIndex != controllables.size(); ++cIndex) {
    PresetCollection &pc = management.AddPresetCollection();
    pc.SetName(destination.GetAvailableName("Colourpreset"));
    destination.Add(pc);
    size_t colorIndex = cIndex % colors.size();
    addColorPresets(management, *controllables[cIndex], pc, colors[colorIndex],
                    deduction);
    management.AddSourceValue(pc, 0);
  }
}

Chase &AutoDesign::MakeRunningLight(
    Management &management, Folder &destination,
    const std::vector<class Controllable *> &controllables,
    const std::vector<class Color> &colors, const ColorDeduction &deduction,
    RunType runType) {
  Chase &chase = management.AddChase();
  chase.SetName(destination.GetAvailableName("Runchase"));
  destination.Add(chase);
  management.AddSourceValue(chase, 0);
  Sequence &seq = chase.GetSequence();
  size_t frames;
  if (runType == InwardRun || runType == OutwardRun)
    frames = (colors.size() + 1) / 2;
  else
    frames = colors.size();
  std::vector<size_t> pos;
  if (runType == RandomRun) {
    pos.resize(frames);
    for (size_t i = 0; i != frames; ++i) pos[i] = i;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::shuffle(pos.begin(), pos.end(), mt);
  }
  for (size_t frameIndex = 0; frameIndex != frames; ++frameIndex) {
    size_t nFixInPattern = 1;
    if ((runType == InwardRun &&
         (frameIndex != frames - 1 || colors.size() % 2 == 0)) ||
        (runType == OutwardRun && (frameIndex != 0 || colors.size() % 2 == 0)))
      nFixInPattern = 2;

    PresetCollection &pc = management.AddPresetCollection();
    pc.SetName(destination.GetAvailableName(chase.Name() + "_"));
    destination.Add(pc);
    // If there are less colours given than fixtures, the sequence is repeated
    // several times. This loop is for that purpose.
    for (size_t patternIndex = 0;
         patternIndex <
         (controllables.size() + colors.size() - 1) / colors.size();
         ++patternIndex) {
      for (size_t fixInPatIndex = 0; fixInPatIndex != nFixInPattern;
           ++fixInPatIndex) {
        size_t cIndex = 0;
        switch (runType) {
          case IncreasingRun:
          case BackAndForthRun:
            cIndex = frameIndex + patternIndex * colors.size();
            break;
          case DecreasingRun:
            cIndex = frames - frameIndex - 1 + patternIndex * colors.size();
            break;
          case RandomRun:
            cIndex = pos[frameIndex] + patternIndex * colors.size();
            break;
          case InwardRun:
            if (fixInPatIndex == 0)
              cIndex = frameIndex + patternIndex * colors.size();
            else
              cIndex = (colors.size() - frameIndex - 1) +
                       patternIndex * colors.size();
            break;
          case OutwardRun:
            if (fixInPatIndex == 0)
              cIndex = frames - frameIndex - 1 + patternIndex * colors.size();
            else
              cIndex = frames + frameIndex + patternIndex * colors.size();
            break;
        }
        if (cIndex < controllables.size()) {
          size_t colourIndex = cIndex % colors.size();
          addColorPresets(management, *controllables[cIndex], pc,
                          colors[colourIndex], deduction);
        }
      }
    }
    seq.Add(pc, 0);
    management.AddSourceValue(pc, 0);
  }
  if (runType == BackAndForthRun) {
    for (size_t i = 2; i < colors.size(); ++i)
      seq.Add(*seq.List()[colors.size() - i].GetControllable(), 0);
  }
  return chase;
}

Chase &AutoDesign::MakeColorVariation(
    class Management &management, Folder &destination,
    const std::vector<class Controllable *> &controllables,
    const std::vector<class Color> &colors, const ColorDeduction &deduction,
    double variation) {
  Chase &chase = management.AddChase();
  chase.SetName(destination.GetAvailableName("Colorvar"));
  destination.Add(chase);
  management.AddSourceValue(chase, 0);
  Sequence &seq = chase.GetSequence();
  std::random_device rd;
  std::mt19937 rnd(rd());
  std::normal_distribution<double> distribution(0.0, variation);
  for (size_t chaseIndex = 0; chaseIndex != colors.size(); ++chaseIndex) {
    PresetCollection &pc = management.AddPresetCollection();
    pc.SetName(destination.GetAvailableName(chase.Name() + "_"));
    destination.Add(pc);
    for (Controllable *c : controllables) {
      double redVar = round(distribution(rnd)),
             greenVar = round(distribution(rnd)),
             blueVar = round(distribution(rnd));
      Color color = colors[chaseIndex];
      Color randomizedColor(
          std::max<double>(0.0,
                           std::min<double>(double(color.Red()) + redVar, 255)),
          std::max<double>(
              0.0, std::min<double>(double(color.Green()) + greenVar, 255)),
          std::max<double>(
              0.0, std::min<double>(double(color.Blue()) + blueVar, 255)));
      addColorPresets(management, *c, pc, randomizedColor, deduction);
    }
    seq.Add(pc, 0);
    management.AddSourceValue(pc, 0);
  }
  return chase;
}

Chase &AutoDesign::MakeColorShift(
    Management &management, Folder &destination,
    const std::vector<class Controllable *> &controllables,
    const std::vector<Color> &colors, const ColorDeduction &deduction,
    ShiftType shiftType) {
  Chase &chase = management.AddChase();
  chase.SetName(destination.GetAvailableName("Colourshift"));
  destination.Add(chase);
  management.AddSourceValue(chase, 0);
  Sequence &seq = chase.GetSequence();
  size_t frames = colors.size();
  std::vector<std::vector<size_t>> pos(frames);
  std::random_device rd;
  std::mt19937 mt(rd());
  for (size_t frameIndex = 0; frameIndex != frames; ++frameIndex) {
    if (shiftType == RandomShift) {
      pos[frameIndex].resize(frames);
      bool duplicate;
      do {
        for (size_t i = 0; i != pos[frameIndex].size(); ++i)
          pos[frameIndex][i] = i;
        std::shuffle(pos[frameIndex].begin(), pos[frameIndex].end(), mt);
        duplicate = false;
        // Check whether previous frames are equal to the new frame
        for (size_t i = 0; i != frameIndex; ++i)
          duplicate = duplicate || pos[i] == pos[frameIndex];
        // Check whether all fixtures are switched to a new position
        // (if all colours are different, this guarantees that the
        //  fixture changes colour)
        if (frameIndex != 0) {
          for (size_t i = 0; i != pos[frameIndex].size(); ++i)
            duplicate =
                duplicate || pos[frameIndex][i] == pos[frameIndex - 1][i];
        }
        // For the last frame, also check whether all positions are different
        // compared to the first frame.
        if (frameIndex == frames - 1) {
          for (size_t i = 0; i != pos[frameIndex].size(); ++i)
            duplicate = duplicate || pos[frameIndex][i] == pos[0][i];
        }
      } while (duplicate);
    }

    PresetCollection &pc = management.AddPresetCollection();
    pc.SetName(destination.GetAvailableName(chase.Name() + "_"));
    destination.Add(pc);

    for (size_t cIndex = 0; cIndex != controllables.size(); ++cIndex) {
      size_t colourIndex = 0;
      switch (shiftType) {
        case IncreasingShift:
        case BackAndForthShift:
          colourIndex = (cIndex + frames - frameIndex) % frames;
          break;
        case DecreasingShift:
          colourIndex = (cIndex + frameIndex) % frames;
          break;
        case RandomShift:
          colourIndex = pos[frameIndex][cIndex % frames];
          break;
      }
      addColorPresets(management, *controllables[cIndex], pc,
                      colors[colourIndex], deduction);
    }
    seq.Add(pc, 0);
    management.AddSourceValue(pc, 0);
  }
  if (shiftType == BackAndForthShift) {
    for (size_t i = 2; i < frames; ++i)
      seq.Add(*seq.List()[frames - i].GetControllable(), 0);
  }
  return chase;
}

Controllable &AutoDesign::MakeVUMeter(
    Management &management, Folder &destination,
    const std::vector<class Controllable *> &controllables,
    const std::vector<Color> &colors, const ColorDeduction &deduction,
    VUMeterDirection direction) {
  if (colors.size() != controllables.size())
    throw std::runtime_error(
        "Number of colours did not match number of fixtures");
  std::unique_ptr<AudioLevelEffect> audioLevel(new AudioLevelEffect());
  audioLevel->SetName(destination.GetAvailableName("VUMeter"));
  Effect &newAudioLevel =
      management.AddEffect(std::move(audioLevel), destination);
  for (size_t inp = 0; inp != newAudioLevel.NInputs(); ++inp)
    management.AddSourceValue(newAudioLevel, inp);
  size_t nLevels;
  if (direction == VUInward || direction == VUOutward)
    nLevels = (controllables.size() + 1) / 2;
  else
    nLevels = controllables.size();
  for (size_t level = 0; level != nLevels; ++level) {
    std::unique_ptr<ThresholdEffect> threshold(new ThresholdEffect());
    threshold->SetLowerStartLimit(((1 << 24) - 1) * level / nLevels);
    threshold->SetLowerEndLimit(((1 << 24) - 1) * (level + 1) / nLevels);
    threshold->SetName(
        destination.GetAvailableName(newAudioLevel.Name() + "_Thr"));
    Effect &newEffect = management.AddEffect(std::move(threshold), destination);
    for (size_t inp = 0; inp != newEffect.NInputs(); ++inp)
      management.AddSourceValue(newEffect, inp);

    size_t nFixInLevel = 1;
    if ((direction == VUInward &&
         (level != nLevels - 1 || controllables.size() % 2 == 0)) ||
        (direction == VUOutward &&
         (level != 0 || controllables.size() % 2 == 0)))
      nFixInLevel = 2;

    PresetCollection &pc = management.AddPresetCollection();
    pc.SetName(destination.GetAvailableName(newAudioLevel.Name() + "_Set"));
    destination.Add(pc);
    for (size_t fixInLevel = 0; fixInLevel != nFixInLevel; ++fixInLevel) {
      size_t fixIndex = 0;
      if (fixInLevel == 0) {
        switch (direction) {
          case VUIncreasing:
          case VUInward:
            fixIndex = level;
            break;
          case VUOutward:
          case VUDecreasing:
            fixIndex = nLevels - level - 1;
            break;
        }
      } else {
        if (direction == VUInward)
          fixIndex = controllables.size() - level - 1;
        else  // VUOutward
          fixIndex = nLevels + level;
      }
      addColorPresets(management, *controllables[fixIndex], pc,
                      colors[fixIndex], deduction);
    }
    management.AddSourceValue(pc, 0);
    newEffect.AddConnection(pc, 0);
    newAudioLevel.AddConnection(newEffect, 0);
  }
  return newAudioLevel;
}

Chase &AutoDesign::MakeIncreasingChase(
    Management &management, Folder &destination,
    const std::vector<class Controllable *> &controllables,
    const std::vector<class Color> &colors, const ColorDeduction &deduction,
    IncreasingType incType) {
  if (colors.size() != controllables.size())
    throw std::runtime_error(
        "Number of controllables does not match number of provided colours");
  Chase &chase = management.AddChase();
  chase.SetName(destination.GetAvailableName("Increasing chase"));
  destination.Add(chase);
  management.AddSourceValue(chase, 0);
  Sequence &seq = chase.GetSequence();

  size_t nFix = controllables.size();
  for (size_t frameIndex = 0; frameIndex != nFix * 2; ++frameIndex) {
    size_t startFixture = 0, endFixture = 0;
    if (frameIndex < controllables.size())  // building up
    {
      switch (incType) {
        case IncForward:
        case IncForwardReturn:
          startFixture = 0;
          endFixture = frameIndex;
          break;
        case IncBackward:
        case IncBackwardReturn:
          startFixture = nFix - frameIndex;
          endFixture = nFix;
          break;
      }
    } else {
      switch (incType) {
        case IncForward:
        case IncBackwardReturn:
          startFixture = frameIndex - nFix;
          endFixture = nFix;
          break;
        case IncBackward:
        case IncForwardReturn:
          startFixture = 0;
          endFixture = (nFix * 2 - frameIndex);
          break;
      }
    }

    PresetCollection &pc = management.AddPresetCollection();
    pc.SetName(destination.GetAvailableName(chase.Name() + "_"));
    destination.Add(pc);

    for (size_t i = startFixture; i != endFixture; ++i) {
      addColorPresets(management, *controllables[i], pc, colors[i], deduction);
    }
    seq.Add(pc, 0);
    management.AddSourceValue(pc, 0);
  }
  return chase;
}

}  // namespace glight::theatre
