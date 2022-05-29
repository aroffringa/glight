#include "fixturetype.h"
#include "fixture.h"

FixtureType::FixtureType(StockFixture stock_fixture)
    : FolderObject(StockName(stock_fixture)), _class(FixtureClass::Par) {
  switch (stock_fixture) {
    case StockFixture::Light1Ch:
      _functions.emplace_back(0, FunctionType::Master, false, 0);
      break;
    case StockFixture::RGBLight3Ch:
      _functions.emplace_back(0, FunctionType::Red, false, 0);
      _functions.emplace_back(1, FunctionType::Green, false, 0);
      _functions.emplace_back(2, FunctionType::Blue, false, 0);
      break;
    case StockFixture::RGBLight4Ch:
      _functions.emplace_back(0, FunctionType::Red, false, 0);
      _functions.emplace_back(1, FunctionType::Green, false, 0);
      _functions.emplace_back(2, FunctionType::Blue, false, 0);
      _functions.emplace_back(3, FunctionType::Master, false, 0);
      break;
    case StockFixture::RGBALight4Ch:
      _functions.emplace_back(0, FunctionType::Red, false, 0);
      _functions.emplace_back(1, FunctionType::Green, false, 0);
      _functions.emplace_back(2, FunctionType::Blue, false, 0);
      _functions.emplace_back(3, FunctionType::Amber, false, 0);
      break;
    case StockFixture::RGBALight5Ch:
      _functions.emplace_back(0, FunctionType::Red, false, 0);
      _functions.emplace_back(1, FunctionType::Green, false, 0);
      _functions.emplace_back(2, FunctionType::Blue, false, 0);
      _functions.emplace_back(3, FunctionType::Amber, false, 0);
      _functions.emplace_back(4, FunctionType::Master, false, 0);
      break;
    case StockFixture::RGBWLight4Ch:
      _functions.emplace_back(0, FunctionType::Red, false, 0);
      _functions.emplace_back(1, FunctionType::Green, false, 0);
      _functions.emplace_back(2, FunctionType::Blue, false, 0);
      _functions.emplace_back(3, FunctionType::White, false, 0);
      break;
    case StockFixture::RGBUVLight4Ch:
      _functions.emplace_back(0, FunctionType::Red, false, 0);
      _functions.emplace_back(1, FunctionType::Green, false, 0);
      _functions.emplace_back(2, FunctionType::Blue, false, 0);
      _functions.emplace_back(3, FunctionType::UV, false, 0);
      break;
    case StockFixture::RGBAWLight5Ch:
      _functions.emplace_back(0, FunctionType::Red, false, 0);
      _functions.emplace_back(1, FunctionType::Green, false, 0);
      _functions.emplace_back(2, FunctionType::Blue, false, 0);
      _functions.emplace_back(3, FunctionType::Amber, false, 0);
      _functions.emplace_back(4, FunctionType::White, false, 0);
      break;
    case StockFixture::RGBAWUVLight6Ch:
      _functions.emplace_back(0, FunctionType::Red, false, 0);
      _functions.emplace_back(1, FunctionType::Green, false, 0);
      _functions.emplace_back(2, FunctionType::Blue, false, 0);
      _functions.emplace_back(3, FunctionType::Amber, false, 0);
      _functions.emplace_back(4, FunctionType::White, false, 0);
      _functions.emplace_back(5, FunctionType::UV, false, 0);
      break;
    case StockFixture::CWWW2Ch:
      _functions.emplace_back(0, FunctionType::ColdWhite, false, 0);
      _functions.emplace_back(1, FunctionType::WarmWhite, false, 0);
      break;
    case StockFixture::CWWW4Ch:
      _functions.emplace_back(0, FunctionType::Master, false, 0);
      _functions.emplace_back(1, FunctionType::ColdWhite, false, 0);
      _functions.emplace_back(2, FunctionType::WarmWhite, false, 0);
      _functions.emplace_back(3, FunctionType::Strobe, false, 0);
      break;
    case StockFixture::CWWWA3Ch:
      _functions.emplace_back(0, FunctionType::ColdWhite, false, 0);
      _functions.emplace_back(1, FunctionType::WarmWhite, false, 0);
      _functions.emplace_back(2, FunctionType::Amber, false, 0);
      break;
    case StockFixture::UVLight3Ch:
      _functions.emplace_back(0, FunctionType::UV, false, 0);
      _functions.emplace_back(1, FunctionType::Strobe, false, 0);
      _functions.emplace_back(2, FunctionType::Pulse, false, 0);
      break;
    case StockFixture::H2ODMXPro:
      _functions.emplace_back(0, FunctionType::Master, false, 0);
      _functions.emplace_back(1, FunctionType::Rotation, false, 0);
      _functions.emplace_back(2, FunctionType::ColorMacro, false, 0);
      break;
    case StockFixture::AyraTDCSunrise:
      _functions.emplace_back(0, FunctionType::Master, false, 0);
      _functions.emplace_back(1, FunctionType::Red, false, 0);
      _functions.emplace_back(2, FunctionType::Green, false, 0);
      _functions.emplace_back(3, FunctionType::Blue, false, 0);
      _functions.emplace_back(4, FunctionType::Strobe, false, 0);
      _functions.emplace_back(5, FunctionType::Rotation, false, 0);
      _functions.emplace_back(6, FunctionType::ColorMacro, false, 0);
      break;
    case StockFixture::RGB_ADJ_6CH:
      _functions.emplace_back(0, FunctionType::Red, false, 0);
      _functions.emplace_back(1, FunctionType::Green, false, 0);
      _functions.emplace_back(2, FunctionType::Blue, false, 0);
      _functions.emplace_back(3, FunctionType::ColorMacro, false, 0);
      _functions.emplace_back(4, FunctionType::Strobe, false, 0);
      _functions.emplace_back(5, FunctionType::Pulse, false, 0);
      break;
    case StockFixture::RGB_ADJ_7CH:
      _functions.emplace_back(0, FunctionType::Red, false, 0);
      _functions.emplace_back(1, FunctionType::Green, false, 0);
      _functions.emplace_back(2, FunctionType::Blue, false, 0);
      _functions.emplace_back(3, FunctionType::ColorMacro, false, 0);
      _functions.emplace_back(4, FunctionType::Strobe, false, 0);
      _functions.emplace_back(5, FunctionType::Pulse, false, 0);
      _functions.emplace_back(6, FunctionType::Master, false, 0);
      break;
    case StockFixture::BT_VINTAGE_5CH:
      _functions.emplace_back(0, FunctionType::White, false, 0);
      _functions.emplace_back(1, FunctionType::Master, false, 0);
      _functions.emplace_back(2, FunctionType::Red, false, 1);
      _functions.emplace_back(3, FunctionType::Green, false, 1);
      _functions.emplace_back(4, FunctionType::Blue, false, 1);
      _class = FixtureClass::RingedPar;
      break;
    case StockFixture::BT_VINTAGE_6CH:
      _functions.emplace_back(0, FunctionType::White, false, 0);
      _functions.emplace_back(1, FunctionType::Master, false, 0);
      _functions.emplace_back(2, FunctionType::Strobe, false, 0);
      _functions.emplace_back(3, FunctionType::Red, false, 1);
      _functions.emplace_back(4, FunctionType::Green, false, 1);
      _functions.emplace_back(5, FunctionType::Blue, false, 1);
      _class = FixtureClass::RingedPar;
      break;
    case StockFixture::BT_VINTAGE_7CH:
      _functions.emplace_back(0, FunctionType::White, false, 0);
      _functions.emplace_back(1, FunctionType::Master, false, 0);
      _functions.emplace_back(2, FunctionType::Strobe, false, 0);
      _functions.emplace_back(3, FunctionType::Red, false, 1);
      _functions.emplace_back(4, FunctionType::Green, false, 1);
      _functions.emplace_back(5, FunctionType::Blue, false, 1);
      _functions.emplace_back(6, FunctionType::ColorMacro, false, 1);
      _class = FixtureClass::RingedPar;
      break;
    case StockFixture::RGBLight6Ch_16bit:
      _functions.emplace_back(0, FunctionType::Red, true, 0);
      _functions.emplace_back(2, FunctionType::Green, true, 0);
      _functions.emplace_back(4, FunctionType::Blue, true, 0);
      break;
  }
  UpdateFunctions();
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

void FixtureType::UpdateFunctions() {
  std::array<unsigned, 3> max_values;
  max_values[0] = 0;
  max_values[1] = 0;
  max_values[2] = 0;
  for (const FixtureTypeFunction &f : _functions) {
    const Color c = GetFunctionColor(f.Type());
    max_values[0] += c.Red();
    max_values[1] += c.Green();
    max_values[2] += c.Blue();
  }
  scaling_value_ = std::max(std::max(max_values[0], max_values[1]),
                            std::max(max_values[2], 1u));
}

Color FixtureType::GetColor(const Fixture &fixture,
                            const ValueSnapshot &snapshot,
                            size_t shapeIndex) const {
  unsigned red = 0;
  unsigned green = 0;
  unsigned blue = 0;
  unsigned master = 255;
  for (size_t i = 0; i != _functions.size(); ++i) {
    if (_functions[i].Shape() == shapeIndex) {
      0;
      115;
      0c const unsigned channel_value =
          fixture.Functions()[i]->GetValue(snapshot);
      if (_functions[i].type == FunctionType::Master) {
        master = channel_value;
      } else {
        const Color c = GetFunctionColor(_functions[i].Type()) * channel_value;
        red += c.Red();
        green += c.Green();
        blue += c.Blue();
      }
    }
  }
  return Color(red * master / scaling_value_, green * master / scaling_value_,
               blue * master / scaling_value_);
}

int FixtureType::GetRotationSpeed(const Fixture &fixture,
                                  const ValueSnapshot &snapshot) const {
  switch (_class) {
    default:
      return 0;
      /*
      case StockFixture::H2ODMXPro: {
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
      case StockFixture::AyraTDCSunrise: {
        const int rotation = fixture.Functions()[5]->GetValue(snapshot);
        const int maxSpeed = (1 << 24) / 100;  // 1 times per second
        if (rotation <= 5)
          return 0;
        else if (rotation <= 127) {
          return 0;  // this should be static positioning
        } else {
          return (rotation - 127) * maxSpeed / 128;
        }
      }*/
  }
}
