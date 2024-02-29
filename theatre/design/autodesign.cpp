#include "autodesign.h"

#include "colorpreset.h"

#include "theatre/chase.h"
#include "theatre/color.h"
#include "theatre/controllable.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"
#include "theatre/sequence.h"

#include "theatre/effects/audioleveleffect.h"
#include "theatre/effects/flickereffect.h"
#include "theatre/effects/thresholdeffect.h"

#include <algorithm>
#include <random>

namespace glight::theatre {

Chase &AutoDesign::MakeRunningLight(
    Management &management, Folder &destination,
    const std::vector<Controllable *> &controllables,
    const std::vector<ColorOrVariable> &colors, const ColorDeduction &deduction,
    RunType runType) {
  Chase &chase = management.AddChase();
  chase.SetName(destination.GetAvailableName("Runchase"));
  destination.Add(chase);
  management.AddSourceValue(chase, 0);
  Sequence &seq = chase.GetSequence();
  size_t frames;
  if (runType == RunType::InwardRun || runType == RunType::OutwardRun)
    frames = (colors.size() + 1) / 2;
  else
    frames = colors.size();
  std::vector<size_t> pos;
  if (runType == RunType::RandomRun) {
    pos.resize(frames);
    for (size_t i = 0; i != frames; ++i) pos[i] = i;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::shuffle(pos.begin(), pos.end(), mt);
  }
  for (size_t frameIndex = 0; frameIndex != frames; ++frameIndex) {
    size_t nFixInPattern = 1;
    if ((runType == RunType::InwardRun &&
         (frameIndex != frames - 1 || colors.size() % 2 == 0)) ||
        (runType == RunType::OutwardRun &&
         (frameIndex != 0 || colors.size() % 2 == 0)))
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
          case RunType::IncreasingRun:
          case RunType::BackAndForthRun:
            cIndex = frameIndex + patternIndex * colors.size();
            break;
          case RunType::DecreasingRun:
            cIndex = frames - frameIndex - 1 + patternIndex * colors.size();
            break;
          case RunType::RandomRun:
            cIndex = pos[frameIndex] + patternIndex * colors.size();
            break;
          case RunType::InwardRun:
            if (fixInPatIndex == 0)
              cIndex = frameIndex + patternIndex * colors.size();
            else
              cIndex = (colors.size() - frameIndex - 1) +
                       patternIndex * colors.size();
            break;
          case RunType::OutwardRun:
            if (fixInPatIndex == 0)
              cIndex = frames - frameIndex - 1 + patternIndex * colors.size();
            else
              cIndex = frames + frameIndex + patternIndex * colors.size();
            break;
        }
        if (cIndex < controllables.size()) {
          size_t colourIndex = cIndex % colors.size();
          AddPresetValue(management, *controllables[cIndex], pc,
                         colors[colourIndex], deduction);
        }
      }
    }
    seq.Add(pc, 0);
    management.AddSourceValue(pc, 0);
  }
  if (runType == RunType::BackAndForthRun) {
    for (size_t i = 2; i < colors.size(); ++i)
      seq.Add(*seq.List()[colors.size() - i].GetControllable(), 0);
  }
  return chase;
}

Chase &AutoDesign::MakeColorVariation(
    Management &management, Folder &destination,
    const std::vector<Controllable *> &controllables,
    const std::vector<ColorOrVariable> &colors, const ColorDeduction &deduction,
    double variation) {
  Chase &chase = management.AddChase();
  chase.SetName(destination.GetAvailableName("Colorvar"));
  destination.Add(chase);
  management.AddSourceValue(chase, 0);
  Sequence &seq = chase.GetSequence();
  std::random_device rd;
  std::mt19937 rnd(rd());
  std::normal_distribution<double> distribution(0.0, variation);
  for (const ColorOrVariable &color_or_var : colors) {
    PresetCollection &pc = management.AddPresetCollection();
    pc.SetName(destination.GetAvailableName(chase.Name() + "_"));
    destination.Add(pc);
    for (Controllable *c : controllables) {
      const double redVar = std::round(distribution(rnd));
      const double greenVar = std::round(distribution(rnd));
      const double blueVar = std::round(distribution(rnd));
      if (std::holds_alternative<Color>(color_or_var)) {
        const Color color = std::get<Color>(color_or_var);
        Color randomizedColor(
            std::max<double>(
                0.0, std::min<double>(static_cast<double>(color.Red()) + redVar,
                                      255)),
            std::max<double>(
                0.0, std::min<double>(
                         static_cast<double>(color.Green()) + greenVar, 255)),
            std::max<double>(
                0.0, std::min<double>(
                         static_cast<double>(color.Blue()) + blueVar, 255)));
        AddPresetValue(management, *c, pc, randomizedColor, deduction);
      } else {
        AddPresetValue(management, *c, pc,
                       std::get<VariableEffect *>(color_or_var), deduction);
      }
    }
    seq.Add(pc, 0);
    management.AddSourceValue(pc, 0);
  }
  return chase;
}

Chase &AutoDesign::MakeColorShift(
    Management &management, Folder &destination,
    const std::vector<Controllable *> &controllables,
    const std::vector<ColorOrVariable> &colors, const ColorDeduction &deduction,
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
    if (shiftType == ShiftType::RandomShift) {
      pos[frameIndex].resize(frames);
      bool duplicate = false;
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
        case ShiftType::IncreasingShift:
        case ShiftType::BackAndForthShift:
          colourIndex = (cIndex + frames - frameIndex) % frames;
          break;
        case ShiftType::DecreasingShift:
          colourIndex = (cIndex + frameIndex) % frames;
          break;
        case ShiftType::RandomShift:
          colourIndex = pos[frameIndex][cIndex % frames];
          break;
      }
      AddPresetValue(management, *controllables[cIndex], pc,
                     colors[colourIndex], deduction);
    }
    seq.Add(pc, 0);
    management.AddSourceValue(pc, 0);
  }
  if (shiftType == ShiftType::BackAndForthShift) {
    for (size_t i = 2; i < frames; ++i)
      seq.Add(*seq.List()[frames - i].GetControllable(), 0);
  }
  return chase;
}

Controllable &AutoDesign::MakeVUMeter(
    Management &management, Folder &destination,
    const std::vector<Controllable *> &controllables,
    const std::vector<ColorOrVariable> &colors, const ColorDeduction &deduction,
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
  size_t nLevels = 0;
  if (direction == VUMeterDirection::VUInward ||
      direction == VUMeterDirection::VUOutward)
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
    if ((direction == VUMeterDirection::VUInward &&
         (level != nLevels - 1 || controllables.size() % 2 == 0)) ||
        (direction == VUMeterDirection::VUOutward &&
         (level != 0 || controllables.size() % 2 == 0)))
      nFixInLevel = 2;

    PresetCollection &pc = management.AddPresetCollection();
    pc.SetName(destination.GetAvailableName(newAudioLevel.Name() + "_Set"));
    destination.Add(pc);
    for (size_t fixInLevel = 0; fixInLevel != nFixInLevel; ++fixInLevel) {
      size_t fixIndex = 0;
      if (fixInLevel == 0) {
        switch (direction) {
          case VUMeterDirection::VUIncreasing:
          case VUMeterDirection::VUInward:
            fixIndex = level;
            break;
          case VUMeterDirection::VUOutward:
          case VUMeterDirection::VUDecreasing:
            fixIndex = nLevels - level - 1;
            break;
        }
      } else {
        if (direction == VUMeterDirection::VUInward)
          fixIndex = controllables.size() - level - 1;
        else  // VUOutward
          fixIndex = nLevels + level;
      }
      AddPresetValue(management, *controllables[fixIndex], pc, colors[fixIndex],
                     deduction);
    }
    management.AddSourceValue(pc, 0);
    newEffect.AddConnection(pc, 0);
    newAudioLevel.AddConnection(newEffect, 0);
  }
  return newAudioLevel;
}

Chase &AutoDesign::MakeIncreasingChase(
    Management &management, Folder &destination,
    const std::vector<Controllable *> &controllables,
    const std::vector<ColorOrVariable> &colors, const ColorDeduction &deduction,
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
    size_t startFixture = 0;
    size_t endFixture = 0;
    if (frameIndex < controllables.size())  // building up
    {
      switch (incType) {
        case IncreasingType::IncForward:
        case IncreasingType::IncForwardReturn:
          startFixture = 0;
          endFixture = frameIndex;
          break;
        case IncreasingType::IncBackward:
        case IncreasingType::IncBackwardReturn:
          startFixture = nFix - frameIndex;
          endFixture = nFix;
          break;
      }
    } else {
      switch (incType) {
        case IncreasingType::IncForward:
        case IncreasingType::IncBackwardReturn:
          startFixture = frameIndex - nFix;
          endFixture = nFix;
          break;
        case IncreasingType::IncBackward:
        case IncreasingType::IncForwardReturn:
          startFixture = 0;
          endFixture = (nFix * 2 - frameIndex);
          break;
      }
    }

    PresetCollection &pc = management.AddPresetCollection();
    pc.SetName(destination.GetAvailableName(chase.Name() + "_"));
    destination.Add(pc);

    for (size_t i = startFixture; i != endFixture; ++i) {
      AddPresetValue(management, *controllables[i], pc, colors[i], deduction);
    }
    seq.Add(pc, 0);
    management.AddSourceValue(pc, 0);
  }
  return chase;
}

Effect &AutoDesign::MakeFire(Management &management, Folder &destination,
                             const std::vector<Controllable *> &controllables,
                             const std::vector<ColorOrVariable> &colors,
                             const ColorDeduction &deduction) {
  std::unique_ptr<FlickerEffect> flicker = std::make_unique<FlickerEffect>();
  flicker->SetSpeed(ControlValue::MaxUInt() / 333);
  flicker->SetName(destination.GetAvailableName("Fire"));
  Effect &parent = management.AddEffect(std::move(flicker), destination);
  for (size_t inp = 0; inp != parent.NInputs(); ++inp)
    management.AddSourceValue(parent, inp);
  std::mt19937 mt;
  std::uniform_int_distribution uniform(0, 10);
  for (size_t i = 0; i != controllables.size(); ++i) {
    Controllable *controllable = controllables[i];
    if (colors.size() == 1) {
      PresetCollection &preset = MakeColorPreset(
          management, destination, {controllable}, colors, deduction);
      parent.AddConnection(preset, 0);
    } else {
      Chase &chase =
          MakeColorShift(management, destination, {controllable}, colors,
                         deduction, ShiftType::IncreasingShift);
      parent.AddConnection(chase, 0);
      chase.GetTransition().SetType(TransitionType::Fade);
      chase.GetTransition().SetLengthInMs(300 + i + uniform(mt) * 10);
      chase.GetTrigger().SetDelayInMs(0);
    }
  }
  return parent;
}

}  // namespace glight::theatre
