#include "fixturetype.h"
#include "fixture.h"

#include <array>

FixtureType::FixtureType(StockFixture stock_fixture)
    : FolderObject(StockName(stock_fixture)), class_(FixtureClass::Par) {
  switch (stock_fixture) {
    case StockFixture::Light1Ch:
      functions_.emplace_back(0, FunctionType::Master, false, 0);
      break;
    case StockFixture::RGBLight3Ch:
      functions_.emplace_back(0, FunctionType::Red, false, 0);
      functions_.emplace_back(1, FunctionType::Green, false, 0);
      functions_.emplace_back(2, FunctionType::Blue, false, 0);
      break;
    case StockFixture::RGBLight4Ch:
      functions_.emplace_back(0, FunctionType::Red, false, 0);
      functions_.emplace_back(1, FunctionType::Green, false, 0);
      functions_.emplace_back(2, FunctionType::Blue, false, 0);
      functions_.emplace_back(3, FunctionType::Master, false, 0);
      break;
    case StockFixture::RGBALight4Ch:
      functions_.emplace_back(0, FunctionType::Red, false, 0);
      functions_.emplace_back(1, FunctionType::Green, false, 0);
      functions_.emplace_back(2, FunctionType::Blue, false, 0);
      functions_.emplace_back(3, FunctionType::Amber, false, 0);
      break;
    case StockFixture::RGBALight5Ch:
      functions_.emplace_back(0, FunctionType::Red, false, 0);
      functions_.emplace_back(1, FunctionType::Green, false, 0);
      functions_.emplace_back(2, FunctionType::Blue, false, 0);
      functions_.emplace_back(3, FunctionType::Amber, false, 0);
      functions_.emplace_back(4, FunctionType::Master, false, 0);
      break;
    case StockFixture::RGBWLight4Ch:
      functions_.emplace_back(0, FunctionType::Red, false, 0);
      functions_.emplace_back(1, FunctionType::Green, false, 0);
      functions_.emplace_back(2, FunctionType::Blue, false, 0);
      functions_.emplace_back(3, FunctionType::White, false, 0);
      break;
    case StockFixture::RGBUVLight4Ch:
      functions_.emplace_back(0, FunctionType::Red, false, 0);
      functions_.emplace_back(1, FunctionType::Green, false, 0);
      functions_.emplace_back(2, FunctionType::Blue, false, 0);
      functions_.emplace_back(3, FunctionType::UV, false, 0);
      break;
    case StockFixture::RGBAWLight5Ch:
      functions_.emplace_back(0, FunctionType::Red, false, 0);
      functions_.emplace_back(1, FunctionType::Green, false, 0);
      functions_.emplace_back(2, FunctionType::Blue, false, 0);
      functions_.emplace_back(3, FunctionType::Amber, false, 0);
      functions_.emplace_back(4, FunctionType::White, false, 0);
      break;
    case StockFixture::RGBAWUVLight6Ch:
      functions_.emplace_back(0, FunctionType::Red, false, 0);
      functions_.emplace_back(1, FunctionType::Green, false, 0);
      functions_.emplace_back(2, FunctionType::Blue, false, 0);
      functions_.emplace_back(3, FunctionType::Amber, false, 0);
      functions_.emplace_back(4, FunctionType::White, false, 0);
      functions_.emplace_back(5, FunctionType::UV, false, 0);
      break;
    case StockFixture::CWWW2Ch:
      functions_.emplace_back(0, FunctionType::ColdWhite, false, 0);
      functions_.emplace_back(1, FunctionType::WarmWhite, false, 0);
      break;
    case StockFixture::CWWW4Ch:
      functions_.emplace_back(0, FunctionType::Master, false, 0);
      functions_.emplace_back(1, FunctionType::ColdWhite, false, 0);
      functions_.emplace_back(2, FunctionType::WarmWhite, false, 0);
      functions_.emplace_back(3, FunctionType::Strobe, false, 0);
      break;
    case StockFixture::CWWWA3Ch:
      functions_.emplace_back(0, FunctionType::ColdWhite, false, 0);
      functions_.emplace_back(1, FunctionType::WarmWhite, false, 0);
      functions_.emplace_back(2, FunctionType::Amber, false, 0);
      break;
    case StockFixture::UVLight3Ch:
      functions_.emplace_back(0, FunctionType::UV, false, 0);
      functions_.emplace_back(1, FunctionType::Strobe, false, 0);
      functions_.emplace_back(2, FunctionType::Pulse, false, 0);
      break;
    case StockFixture::H2ODMXPro: {
      functions_.emplace_back(0, FunctionType::Master, false, 0);
      functions_.emplace_back(1, FunctionType::Rotation, false, 0);
      functions_.emplace_back(2, FunctionType::ColorMacro, false, 0);
      std::vector<RotationParameters::Range> &ranges =
          functions_[1].GetRotationParameters().GetRanges();
      constexpr int max_speed = (1 << 24) / 100;  // 1 times per second
      ranges.emplace_back(9, 121, 0, max_speed);
      ranges.emplace_back(134, 256, 0, -max_speed);
    } break;
    case StockFixture::AyraTDCSunrise: {
      functions_.emplace_back(0, FunctionType::Master, false, 0);
      functions_.emplace_back(1, FunctionType::Red, false, 0);
      functions_.emplace_back(2, FunctionType::Green, false, 0);
      functions_.emplace_back(3, FunctionType::Blue, false, 0);
      functions_.emplace_back(4, FunctionType::Strobe, false, 0);
      functions_.emplace_back(5, FunctionType::Rotation, false, 0);
      functions_.emplace_back(6, FunctionType::ColorMacro, false, 0);
      constexpr int max_speed = (1 << 24) / 100;  // 1 times per second
      std::vector<RotationParameters::Range> &ranges =
          functions_[5].GetRotationParameters().GetRanges();
      // [ 5 - 128 ) is static positioning
      ranges.emplace_back(128, 256, -max_speed, max_speed);
    } break;
    case StockFixture::RGB_ADJ_6CH:
      functions_.emplace_back(0, FunctionType::Red, false, 0);
      functions_.emplace_back(1, FunctionType::Green, false, 0);
      functions_.emplace_back(2, FunctionType::Blue, false, 0);
      functions_.emplace_back(3, FunctionType::ColorMacro, false, 0);
      functions_.emplace_back(4, FunctionType::Strobe, false, 0);
      functions_.emplace_back(5, FunctionType::Pulse, false, 0);
      break;
    case StockFixture::RGB_ADJ_7CH:
      functions_.emplace_back(0, FunctionType::Red, false, 0);
      functions_.emplace_back(1, FunctionType::Green, false, 0);
      functions_.emplace_back(2, FunctionType::Blue, false, 0);
      functions_.emplace_back(3, FunctionType::ColorMacro, false, 0);
      functions_.emplace_back(4, FunctionType::Strobe, false, 0);
      functions_.emplace_back(5, FunctionType::Pulse, false, 0);
      functions_.emplace_back(6, FunctionType::Master, false, 0);
      break;
    case StockFixture::BT_VINTAGE_5CH:
      functions_.emplace_back(0, FunctionType::White, false, 0);
      functions_.emplace_back(1, FunctionType::Master, false, 0);
      functions_.emplace_back(2, FunctionType::Red, false, 1);
      functions_.emplace_back(3, FunctionType::Green, false, 1);
      functions_.emplace_back(4, FunctionType::Blue, false, 1);
      class_ = FixtureClass::RingedPar;
      break;
    case StockFixture::BT_VINTAGE_6CH:
      functions_.emplace_back(0, FunctionType::White, false, 0);
      functions_.emplace_back(1, FunctionType::Master, false, 0);
      functions_.emplace_back(2, FunctionType::Strobe, false, 0);
      functions_.emplace_back(3, FunctionType::Red, false, 1);
      functions_.emplace_back(4, FunctionType::Green, false, 1);
      functions_.emplace_back(5, FunctionType::Blue, false, 1);
      class_ = FixtureClass::RingedPar;
      break;
    case StockFixture::BT_VINTAGE_7CH:
      functions_.emplace_back(0, FunctionType::White, false, 0);
      functions_.emplace_back(1, FunctionType::Master, false, 0);
      functions_.emplace_back(2, FunctionType::Strobe, false, 0);
      functions_.emplace_back(3, FunctionType::Red, false, 1);
      functions_.emplace_back(4, FunctionType::Green, false, 1);
      functions_.emplace_back(5, FunctionType::Blue, false, 1);
      functions_.emplace_back(6, FunctionType::ColorMacro, false, 1);
      class_ = FixtureClass::RingedPar;
      break;
    case StockFixture::RGBLight6Ch_16bit:
      functions_.emplace_back(0, FunctionType::Red, true, 0);
      functions_.emplace_back(2, FunctionType::Green, true, 0);
      functions_.emplace_back(4, FunctionType::Blue, true, 0);
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
  for (const FixtureTypeFunction &f : functions_) {
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
  for (size_t i = 0; i != functions_.size(); ++i) {
    if (functions_[i].Shape() == shapeIndex) {
      const unsigned channel_value = fixture.Functions()[i]->GetValue(snapshot);
      if (functions_[i].Type() == FunctionType::Master) {
        master = channel_value;
      } else {
        const Color c = GetFunctionColor(functions_[i].Type()) * channel_value;
        red += c.Red();
        green += c.Green();
        blue += c.Blue();
      }
    }
  }
  return Color(red * master / scaling_value_, green * master / scaling_value_,
               blue * master / scaling_value_);
}

#include <iostream>
int FixtureType::GetRotationSpeed(const Fixture &fixture,
                                  const ValueSnapshot &snapshot,
                                  size_t shape_index) const {
  for (size_t i = 0; i != functions_.size(); ++i) {
    if (functions_[i].Shape() == shape_index &&
        functions_[i].Type() == FunctionType::Rotation) {
      const unsigned channel_value = fixture.Functions()[i]->GetValue(snapshot);
      std::cout << functions_[i].GetRotationParameters().GetRanges().size()
                << " " << channel_value << " "
                << functions_[i].GetRotationParameters().GetSpeed(channel_value)
                << '\n';
      return functions_[i].GetRotationParameters().GetSpeed(channel_value);
    }
  }
  return 0;
}
