#include "fixturetype.h"
#include "fixture.h"

FixtureType::FixtureType(enum FixtureClass fixtureClass)
    : FolderObject(ClassName(fixtureClass)), _class(fixtureClass) {
  switch (fixtureClass) {
    case FixtureType::Light1Ch:
      _functionTypes.emplace_back(FunctionType::Master);
      break;
    case FixtureType::RGBLight3Ch:
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      break;
    case FixtureType::RGBLight4Ch:
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      _functionTypes.emplace_back(FunctionType::Master);
      break;
    case FixtureType::RGBALight4Ch:
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      _functionTypes.emplace_back(FunctionType::Amber);
      break;
    case FixtureType::RGBALight5Ch:
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      _functionTypes.emplace_back(FunctionType::Amber);
      _functionTypes.emplace_back(FunctionType::Master);
      break;
    case FixtureType::RGBWLight4Ch:
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      _functionTypes.emplace_back(FunctionType::White);
      break;
    case FixtureType::RGBUVLight4Ch:
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      _functionTypes.emplace_back(FunctionType::UV);
      break;
    case FixtureType::RGBAWUVLight6Ch:
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      _functionTypes.emplace_back(FunctionType::Amber);
      _functionTypes.emplace_back(FunctionType::White);
      _functionTypes.emplace_back(FunctionType::UV);
      break;
    case FixtureType::CWWW2Ch:
      _functionTypes.emplace_back(FunctionType::ColdWhite);
      _functionTypes.emplace_back(FunctionType::WarmWhite);
      break;
    case FixtureType::CWWW4Ch:
      _functionTypes.emplace_back(FunctionType::Master);
      _functionTypes.emplace_back(FunctionType::ColdWhite);
      _functionTypes.emplace_back(FunctionType::WarmWhite);
      _functionTypes.emplace_back(FunctionType::Strobe);
      break;
    case FixtureType::CWWWA3Ch:
      _functionTypes.emplace_back(FunctionType::ColdWhite);
      _functionTypes.emplace_back(FunctionType::WarmWhite);
      _functionTypes.emplace_back(FunctionType::Amber);
      break;
    case FixtureType::UVLight3Ch:
      _functionTypes.emplace_back(FunctionType::UV);
      _functionTypes.emplace_back(FunctionType::Strobe);
      _functionTypes.emplace_back(FunctionType::Pulse);
      break;
    case FixtureType::H2ODMXPro:
      _functionTypes.emplace_back(FunctionType::Master);
      _functionTypes.emplace_back(FunctionType::Rotation);
      _functionTypes.emplace_back(FunctionType::ColorMacro);
      break;
    case FixtureType::RGB_ADJ_6CH:
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      _functionTypes.emplace_back(FunctionType::ColorMacro);
      _functionTypes.emplace_back(FunctionType::Strobe);
      _functionTypes.emplace_back(FunctionType::Pulse);
      break;
    case FixtureType::RGB_ADJ_7CH:
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      _functionTypes.emplace_back(FunctionType::ColorMacro);
      _functionTypes.emplace_back(FunctionType::Strobe);
      _functionTypes.emplace_back(FunctionType::Pulse);
      _functionTypes.emplace_back(FunctionType::Master);
      break;
    case FixtureType::BT_VINTAGE_5CH:
      _functionTypes.emplace_back(FunctionType::White);
      _functionTypes.emplace_back(FunctionType::Master);
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      break;
    case FixtureType::BT_VINTAGE_6CH:
      _functionTypes.emplace_back(FunctionType::White);
      _functionTypes.emplace_back(FunctionType::Master);
      _functionTypes.emplace_back(FunctionType::Strobe);
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      break;
    case FixtureType::BT_VINTAGE_7CH:
      _functionTypes.emplace_back(FunctionType::White);
      _functionTypes.emplace_back(FunctionType::Master);
      _functionTypes.emplace_back(FunctionType::Strobe);
      _functionTypes.emplace_back(FunctionType::Red);
      _functionTypes.emplace_back(FunctionType::Green);
      _functionTypes.emplace_back(FunctionType::Blue);
      _functionTypes.emplace_back(FunctionType::ColorMacro);
      break;
  }
}

Color FixtureType::rgbAdj6chColor(const class Fixture &fixture,
                                  const ValueSnapshot &snapshot) {
  unsigned macro = fixture.Functions()[3]->GetValue(snapshot);
  if (macro < 8) {
    return Color(fixture.Functions()[0]->GetValue(snapshot),
                 fixture.Functions()[1]->GetValue(snapshot),
                 fixture.Functions()[2]->GetValue(snapshot));
  } else {
    return Color::ADJMacroColor(macro);
  }
}

Color FixtureType::GetColor(const Fixture &fixture,
                            const ValueSnapshot &snapshot,
                            size_t shapeIndex) const {
  switch (_class) {
    case Light1Ch:
      return Color::Gray(fixture.Functions()[0]->GetValue(snapshot));
    case RGBLight3Ch:
      return Color(fixture.Functions()[0]->GetValue(snapshot),
                   fixture.Functions()[1]->GetValue(snapshot),
                   fixture.Functions()[2]->GetValue(snapshot));
    case RGBLight4Ch: {
      const unsigned char master = fixture.Functions()[3]->GetValue(snapshot);
      return Color((fixture.Functions()[0]->GetValue(snapshot) * master) / 255,
                   (fixture.Functions()[1]->GetValue(snapshot) * master) / 255,
                   (fixture.Functions()[2]->GetValue(snapshot) * master) / 255);
    }
    case RGBALight4Ch: {
      const unsigned char a = fixture.Functions()[3]->GetValue(snapshot);
      return Color(
          (fixture.Functions()[0]->GetValue(snapshot) + a * 2 / 3) * 3 / 5,
          (fixture.Functions()[1]->GetValue(snapshot) + a / 3) * 3 / 5,
          (fixture.Functions()[2]->GetValue(snapshot) * 3 / 5));
      break;
    }
    case RGBALight5Ch: {
      const unsigned char a = fixture.Functions()[3]->GetValue(snapshot);
      const unsigned char master = fixture.Functions()[4]->GetValue(snapshot);
      return Color(
          ((fixture.Functions()[0]->GetValue(snapshot) + a * 2 / 3) * 3 / 5) *
              master / 255,
          ((fixture.Functions()[1]->GetValue(snapshot) + a / 3) * 3 / 5) *
              master / 255,
          ((fixture.Functions()[2]->GetValue(snapshot) * 3 / 5)) * master /
              255);
      break;
    }
    case RGBWLight4Ch: {
      const unsigned char w = fixture.Functions()[3]->GetValue(snapshot);
      return Color(
          (fixture.Functions()[0]->GetValue(snapshot) + w / 2) * 2 / 3,
          (fixture.Functions()[1]->GetValue(snapshot) + w / 2) * 2 / 3,
          (fixture.Functions()[2]->GetValue(snapshot) + w / 2) * 2 / 3);
      break;
    }
    case RGBUVLight4Ch: {
      const unsigned char uv = fixture.Functions()[3]->GetValue(snapshot);
      return Color((fixture.Functions()[0]->GetValue(snapshot) + uv / 3) / 2,
                   (fixture.Functions()[1]->GetValue(snapshot)) / 2,
                   (fixture.Functions()[2]->GetValue(snapshot) + uv) / 2);
      break;
    }
    case RGBAWUVLight6Ch: {
      const unsigned char r = fixture.Functions()[0]->GetValue(snapshot);
      const unsigned char g = fixture.Functions()[1]->GetValue(snapshot);
      const unsigned char b = fixture.Functions()[2]->GetValue(snapshot);
      const unsigned char a = fixture.Functions()[3]->GetValue(snapshot);
      const unsigned char w = fixture.Functions()[4]->GetValue(snapshot);
      const unsigned char uv = fixture.Functions()[5]->GetValue(snapshot);
      return Color((r + uv / 3 + w + a * 2 / 3) / 3, (g + w + a / 3) / 3,
                   (b + uv + w) / 3);
      break;
    }
    case CWWW2Ch: {
      const unsigned char cw = fixture.Functions()[0]->GetValue(snapshot);
      const unsigned char ww = fixture.Functions()[1]->GetValue(snapshot);
      return Color((228 * cw + 255 * ww) / 483, (228 * cw + 228 * ww) / 483,
                   (255 * cw + 228 * ww) / 483);
    }
    case CWWW4Ch: {
      const unsigned char m = fixture.Functions()[0]->GetValue(snapshot);
      const unsigned char cw = fixture.Functions()[1]->GetValue(snapshot);
      const unsigned char ww = fixture.Functions()[2]->GetValue(snapshot);
      return Color((228 * cw + 255 * ww) / 483, (228 * cw + 228 * ww) / 483,
                   (255 * cw + 228 * ww) / 483) *
             m;
    }
    case CWWWA3Ch: {
      const unsigned char cw = fixture.Functions()[0]->GetValue(snapshot);
      const unsigned char ww = fixture.Functions()[1]->GetValue(snapshot);
      const unsigned char a = fixture.Functions()[2]->GetValue(snapshot);
      return Color((228 * cw + 255 * ww + 170 * a) / 653,
                   (228 * cw + 228 * ww + 85 * a) / 653,
                   (255 * cw + 228 * ww) / 653);
    }
    case UVLight3Ch: {
      const unsigned char m = fixture.Functions()[0]->GetValue(snapshot);
      return Color(m / 3, 0, m);
    }
    case H2ODMXPro: {
      return Color::H20Color(fixture.Functions()[2]->GetValue(snapshot)) *
             fixture.Functions()[0]->GetValue(snapshot);
    }
    case RGB_ADJ_6CH: {
      return rgbAdj6chColor(fixture, snapshot);
    }
    case RGB_ADJ_7CH: {
      Color c = rgbAdj6chColor(fixture, snapshot);
      const unsigned char master = fixture.Functions()[6]->GetValue(snapshot);
      return c * master;
    }
    case BT_VINTAGE_5CH:
    case BT_VINTAGE_6CH: {
      if (shapeIndex == 0)
        return Color(255, 171, 85) * fixture.Functions()[0]->GetValue(snapshot);
      else {
        const unsigned char master = fixture.Functions()[1]->GetValue(snapshot);
        size_t strobe = _class == BT_VINTAGE_5CH ? 0 : 1;
        return master *
               Color(fixture.Functions()[2 + strobe]->GetValue(snapshot),
                     fixture.Functions()[3 + strobe]->GetValue(snapshot),
                     fixture.Functions()[4 + strobe]->GetValue(snapshot));
      }
    }
    case BT_VINTAGE_7CH: {
      if (shapeIndex == 0)
        return Color(255, 171, 85) * fixture.Functions()[0]->GetValue(snapshot);
      else {
        const unsigned char master = fixture.Functions()[1]->GetValue(snapshot);
        const unsigned char colorEffect =
            fixture.Functions()[6]->GetValue(snapshot);
        if (colorEffect < 8)
          return master * Color(fixture.Functions()[3]->GetValue(snapshot),
                                fixture.Functions()[4]->GetValue(snapshot),
                                fixture.Functions()[5]->GetValue(snapshot));
        else
          return master * Color::BTMacroColor(
                              fixture.Functions()[6]->GetValue(snapshot));
      }
    }
  }
  throw std::runtime_error("Unknown fixture class");
}
