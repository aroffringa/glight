#include "fixturetype.h"
#include "fixture.h"

FixtureType::FixtureType(enum FixtureClass fixtureClass)
    : FolderObject(ClassName(fixtureClass)), _class(fixtureClass) {
  switch (fixtureClass) {
    case FixtureType::Light1Ch:
      _functions.emplace_back(0, FunctionType::Master, false);
      break;
    case FixtureType::RGBLight3Ch:
      _functions.emplace_back(0, FunctionType::Red, false);
      _functions.emplace_back(1, FunctionType::Green, false);
      _functions.emplace_back(2, FunctionType::Blue, false);
      break;
    case FixtureType::RGBLight4Ch:
      _functions.emplace_back(0, FunctionType::Red, false);
      _functions.emplace_back(1, FunctionType::Green, false);
      _functions.emplace_back(2, FunctionType::Blue, false);
      _functions.emplace_back(3, FunctionType::Master, false);
      break;
    case FixtureType::RGBALight4Ch:
      _functions.emplace_back(0, FunctionType::Red, false);
      _functions.emplace_back(1, FunctionType::Green, false);
      _functions.emplace_back(2, FunctionType::Blue, false);
      _functions.emplace_back(3, FunctionType::Amber, false);
      break;
    case FixtureType::RGBALight5Ch:
      _functions.emplace_back(0, FunctionType::Red, false);
      _functions.emplace_back(1, FunctionType::Green, false);
      _functions.emplace_back(2, FunctionType::Blue, false);
      _functions.emplace_back(3, FunctionType::Amber, false);
      _functions.emplace_back(4, FunctionType::Master, false);
      break;
    case FixtureType::RGBWLight4Ch:
      _functions.emplace_back(0, FunctionType::Red, false);
      _functions.emplace_back(1, FunctionType::Green, false);
      _functions.emplace_back(2, FunctionType::Blue, false);
      _functions.emplace_back(3, FunctionType::White, false);
      break;
    case FixtureType::RGBUVLight4Ch:
      _functions.emplace_back(0, FunctionType::Red, false);
      _functions.emplace_back(1, FunctionType::Green, false);
      _functions.emplace_back(2, FunctionType::Blue, false);
      _functions.emplace_back(3, FunctionType::UV, false);
      break;
    case FixtureType::RGBAWUVLight6Ch:
      _functions.emplace_back(0, FunctionType::Red, false);
      _functions.emplace_back(1, FunctionType::Green, false);
      _functions.emplace_back(2, FunctionType::Blue, false);
      _functions.emplace_back(3, FunctionType::Amber, false);
      _functions.emplace_back(4, FunctionType::White, false);
      _functions.emplace_back(5, FunctionType::UV, false);
      break;
    case FixtureType::CWWW2Ch:
      _functions.emplace_back(0, FunctionType::ColdWhite, false);
      _functions.emplace_back(1, FunctionType::WarmWhite, false);
      break;
    case FixtureType::CWWW4Ch:
      _functions.emplace_back(0, FunctionType::Master, false);
      _functions.emplace_back(1, FunctionType::ColdWhite, false);
      _functions.emplace_back(2, FunctionType::WarmWhite, false);
      _functions.emplace_back(3, FunctionType::Strobe, false);
      break;
    case FixtureType::CWWWA3Ch:
      _functions.emplace_back(0, FunctionType::ColdWhite, false);
      _functions.emplace_back(1, FunctionType::WarmWhite, false);
      _functions.emplace_back(2, FunctionType::Amber, false);
      break;
    case FixtureType::UVLight3Ch:
      _functions.emplace_back(0, FunctionType::UV, false);
      _functions.emplace_back(1, FunctionType::Strobe, false);
      _functions.emplace_back(2, FunctionType::Pulse, false);
      break;
    case FixtureType::H2ODMXPro:
      _functions.emplace_back(0, FunctionType::Master, false);
      _functions.emplace_back(1, FunctionType::Rotation, false);
      _functions.emplace_back(2, FunctionType::ColorMacro, false);
      break;
    case FixtureType::AyraTDCSunrise:
      _functions.emplace_back(0, FunctionType::Master, false);
      _functions.emplace_back(1, FunctionType::Red, false);
      _functions.emplace_back(2, FunctionType::Green, false);
      _functions.emplace_back(3, FunctionType::Blue, false);
      _functions.emplace_back(4, FunctionType::Strobe, false);
      _functions.emplace_back(5, FunctionType::Rotation, false);
      _functions.emplace_back(6, FunctionType::ColorMacro, false);
      break;
    case FixtureType::RGB_ADJ_6CH:
      _functions.emplace_back(0, FunctionType::Red, false);
      _functions.emplace_back(1, FunctionType::Green, false);
      _functions.emplace_back(2, FunctionType::Blue, false);
      _functions.emplace_back(3, FunctionType::ColorMacro, false);
      _functions.emplace_back(4, FunctionType::Strobe, false);
      _functions.emplace_back(5, FunctionType::Pulse, false);
      break;
    case FixtureType::RGB_ADJ_7CH:
      _functions.emplace_back(0, FunctionType::Red, false);
      _functions.emplace_back(1, FunctionType::Green, false);
      _functions.emplace_back(2, FunctionType::Blue, false);
      _functions.emplace_back(3, FunctionType::ColorMacro, false);
      _functions.emplace_back(4, FunctionType::Strobe, false);
      _functions.emplace_back(5, FunctionType::Pulse, false);
      _functions.emplace_back(6, FunctionType::Master, false);
      break;
    case FixtureType::BT_VINTAGE_5CH:
      _functions.emplace_back(0, FunctionType::White, false);
      _functions.emplace_back(1, FunctionType::Master, false);
      _functions.emplace_back(2, FunctionType::Red, false);
      _functions.emplace_back(3, FunctionType::Green, false);
      _functions.emplace_back(4, FunctionType::Blue, false);
      break;
    case FixtureType::BT_VINTAGE_6CH:
      _functions.emplace_back(0, FunctionType::White, false);
      _functions.emplace_back(1, FunctionType::Master, false);
      _functions.emplace_back(2, FunctionType::Strobe, false);
      _functions.emplace_back(3, FunctionType::Red, false);
      _functions.emplace_back(4, FunctionType::Green, false);
      _functions.emplace_back(5, FunctionType::Blue, false);
      break;
    case FixtureType::BT_VINTAGE_7CH:
      _functions.emplace_back(0, FunctionType::White, false);
      _functions.emplace_back(1, FunctionType::Master, false);
      _functions.emplace_back(2, FunctionType::Strobe, false);
      _functions.emplace_back(3, FunctionType::Red, false);
      _functions.emplace_back(4, FunctionType::Green, false);
      _functions.emplace_back(5, FunctionType::Blue, false);
      _functions.emplace_back(6, FunctionType::ColorMacro, false);
      break;
    case FixtureType::RGBLight6Ch_16bit:
      _functions.emplace_back(0, FunctionType::Red, true);
      _functions.emplace_back(1, FunctionType::Green, true);
      _functions.emplace_back(2, FunctionType::Blue, true);
      break;
  }
}

Color FixtureType::rgbAdj6chColor(const Fixture &fixture,
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
    case AyraTDCSunrise: {
      return Color(fixture.Functions()[1]->GetValue(snapshot),
                   fixture.Functions()[2]->GetValue(snapshot),
                   fixture.Functions()[3]->GetValue(snapshot)) *
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
    case RGBLight6Ch_16bit:
      return Color(fixture.Functions()[0]->GetValue(snapshot),
                   fixture.Functions()[1]->GetValue(snapshot),
                   fixture.Functions()[2]->GetValue(snapshot));
  }
  throw std::runtime_error("Unknown fixture class");
}

int FixtureType::GetRotationSpeed(const Fixture &fixture,
                                  const ValueSnapshot &snapshot) const {
  switch (_class) {
    default:
      return 0;
    case H2ODMXPro: {
      const int rotation = fixture.Functions()[1]->GetValue(snapshot);
      const int maxSpeed = (1 << 24) / 100;  // 1 times per second
      if (rotation <= 9 || (rotation >= 121 && rotation <= 134) ||
          rotation >= 246)
        return 0;
      else if (rotation <= 120) {
        return (121 - rotation) * maxSpeed / 111;
      } else {
        return -(rotation - 134) * maxSpeed / 111;
      }
    }
    case AyraTDCSunrise: {
      const int rotation = fixture.Functions()[5]->GetValue(snapshot);
      const int maxSpeed = (1 << 24) / 100;  // 1 times per second
      if (rotation <= 5)
        return 0;
      else if (rotation <= 127) {
        return 0;  // this should be static positioning
      } else {
        return (rotation - 127) * maxSpeed / 128;
      }
    }
  }
}
