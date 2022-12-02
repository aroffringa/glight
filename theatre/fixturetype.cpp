#include "fixturetype.h"
#include "fixture.h"

#include <array>

namespace glight::theatre {

FixtureType::FixtureType(StockFixture stock_fixture)
    : FolderObject(StockName(stock_fixture)), class_(FixtureClass::Par) {
  switch (stock_fixture) {
    case StockFixture::Light1Ch:
      functions_.emplace_back(FunctionType::Master, 0, false, 0);
      short_name_ = "Light";
      break;
    case StockFixture::RGBLight3Ch:
      functions_.emplace_back(FunctionType::Red, 0, false, 0);
      functions_.emplace_back(FunctionType::Green, 1, false, 0);
      functions_.emplace_back(FunctionType::Blue, 2, false, 0);
      short_name_ = "RGB";
      break;
    case StockFixture::RGBLight4Ch:
      functions_.emplace_back(FunctionType::Red, 0, false, 0);
      functions_.emplace_back(FunctionType::Green, 1, false, 0);
      functions_.emplace_back(FunctionType::Blue, 2, false, 0);
      functions_.emplace_back(FunctionType::Master, 3, false, 0);
      short_name_ = "RGB";
      break;
    case StockFixture::RGBALight4Ch:
      functions_.emplace_back(FunctionType::Red, 0, false, 0);
      functions_.emplace_back(FunctionType::Green, 1, false, 0);
      functions_.emplace_back(FunctionType::Blue, 2, false, 0);
      functions_.emplace_back(FunctionType::Amber, 3, false, 0);
      short_name_ = "RGBA";
      break;
    case StockFixture::RGBALight5Ch:
      functions_.emplace_back(FunctionType::Red, 0, false, 0);
      functions_.emplace_back(FunctionType::Green, 1, false, 0);
      functions_.emplace_back(FunctionType::Blue, 2, false, 0);
      functions_.emplace_back(FunctionType::Amber, 3, false, 0);
      functions_.emplace_back(FunctionType::Master, 4, false, 0);
      short_name_ = "RGBA";
      break;
    case StockFixture::RGBWLight4Ch:
      functions_.emplace_back(FunctionType::Red, 0, false, 0);
      functions_.emplace_back(FunctionType::Green, 1, false, 0);
      functions_.emplace_back(FunctionType::Blue, 2, false, 0);
      functions_.emplace_back(FunctionType::White, 3, false, 0);
      short_name_ = "RGBW";
      break;
    case StockFixture::RGBUVLight4Ch:
      functions_.emplace_back(FunctionType::Red, 0, false, 0);
      functions_.emplace_back(FunctionType::Green, 1, false, 0);
      functions_.emplace_back(FunctionType::Blue, 2, false, 0);
      functions_.emplace_back(FunctionType::UV, 3, false, 0);
      short_name_ = "RGBUV";
      break;
    case StockFixture::RGBAWLight5Ch:
      functions_.emplace_back(FunctionType::Red, 0, false, 0);
      functions_.emplace_back(FunctionType::Green, 1, false, 0);
      functions_.emplace_back(FunctionType::Blue, 2, false, 0);
      functions_.emplace_back(FunctionType::Amber, 3, false, 0);
      functions_.emplace_back(FunctionType::White, 4, false, 0);
      short_name_ = "RGBAW";
      break;
    case StockFixture::RGBAWUVLight6Ch:
      functions_.emplace_back(FunctionType::Red, 0, false, 0);
      functions_.emplace_back(FunctionType::Green, 1, false, 0);
      functions_.emplace_back(FunctionType::Blue, 2, false, 0);
      functions_.emplace_back(FunctionType::Amber, 3, false, 0);
      functions_.emplace_back(FunctionType::White, 4, false, 0);
      functions_.emplace_back(FunctionType::UV, 5, false, 0);
      short_name_ = "RGBAWUV";
      break;
    case StockFixture::CWWW2Ch:
      functions_.emplace_back(FunctionType::ColdWhite, 0, false, 0);
      functions_.emplace_back(FunctionType::WarmWhite, 1, false, 0);
      short_name_ = "CW/WW";
      break;
    case StockFixture::CWWW4Ch:
      functions_.emplace_back(FunctionType::Master, 0, false, 0);
      functions_.emplace_back(FunctionType::ColdWhite, 1, false, 0);
      functions_.emplace_back(FunctionType::WarmWhite, 2, false, 0);
      functions_.emplace_back(FunctionType::Strobe, 3, false, 0);
      short_name_ = "CW/WW";
      break;
    case StockFixture::CWWWA3Ch:
      functions_.emplace_back(FunctionType::ColdWhite, 0, false, 0);
      functions_.emplace_back(FunctionType::WarmWhite, 1, false, 0);
      functions_.emplace_back(FunctionType::Amber, 2, false, 0);
      short_name_ = "CW/WW/A";
      break;
    case StockFixture::UVLight3Ch:
      functions_.emplace_back(FunctionType::UV, 0, false, 0);
      functions_.emplace_back(FunctionType::Strobe, 1, false, 0);
      functions_.emplace_back(FunctionType::Pulse, 2, false, 0);
      short_name_ = "UV";
      break;
    case StockFixture::H2ODMXPro: {
      functions_.emplace_back(FunctionType::Master, 0, false, 0);
      FixtureTypeFunction &rotation =
          functions_.emplace_back(FunctionType::Rotation, 1, false, 0);
      std::vector<RotationParameters::Range> &ranges =
          rotation.GetRotationParameters().GetRanges();
      constexpr int max_speed = (1 << 24) / 100;  // 1 times per second
      ranges.emplace_back(9, 121, 0, max_speed);
      ranges.emplace_back(134, 256, 0, -max_speed);
      FixtureTypeFunction &macro =
          functions_.emplace_back(FunctionType::ColorMacro, 2, false, 0);
      SetH2OMacroParameters(macro.GetMacroParameters());
      short_name_ = "H2O";
    } break;
    case StockFixture::AyraTDCSunrise: {
      functions_.emplace_back(FunctionType::Master, 0, false, 0);
      functions_.emplace_back(FunctionType::Red, 1, false, 0);
      functions_.emplace_back(FunctionType::Green, 2, false, 0);
      functions_.emplace_back(FunctionType::Blue, 3, false, 0);
      functions_.emplace_back(FunctionType::Strobe, 4, false, 0);
      FixtureTypeFunction &rotation =
          functions_.emplace_back(FunctionType::Rotation, 5, false, 0);
      constexpr int max_speed = (1 << 24) / 100;  // 1 times per second
      std::vector<RotationParameters::Range> &ranges =
          rotation.GetRotationParameters().GetRanges();
      // [ 5 - 128 ) is static positioning
      ranges.emplace_back(128, 256, -max_speed, max_speed);
      functions_.emplace_back(FunctionType::ColorMacro, 6, false, 0);
      short_name_ = "TDC Sunr";
      beam_angle_ = 2.0 * M_PI;
      brightness_ = 1.0;
    } break;
    case StockFixture::RGB_ADJ_6CH: {
      functions_.emplace_back(FunctionType::Red, 0, false, 0);
      functions_.emplace_back(FunctionType::Green, 1, false, 0);
      functions_.emplace_back(FunctionType::Blue, 2, false, 0);
      FixtureTypeFunction &macro =
          functions_.emplace_back(FunctionType::ColorMacro, 3, false, 0);
      SetRgbAdj6chMacroParameters(macro.GetMacroParameters());
      functions_.emplace_back(FunctionType::Strobe, 4, false, 0);
      functions_.emplace_back(FunctionType::Pulse, 5, false, 0);
      short_name_ = "ADJ RGB";
    } break;
    case StockFixture::RGB_ADJ_7CH: {
      functions_.emplace_back(FunctionType::Red, 0, false, 0);
      functions_.emplace_back(FunctionType::Green, 1, false, 0);
      functions_.emplace_back(FunctionType::Blue, 2, false, 0);
      FixtureTypeFunction &macro =
          functions_.emplace_back(FunctionType::ColorMacro, 3, false, 0);
      SetRgbAdj6chMacroParameters(macro.GetMacroParameters());
      functions_.emplace_back(FunctionType::Strobe, 4, false, 0);
      functions_.emplace_back(FunctionType::Pulse, 5, false, 0);
      functions_.emplace_back(FunctionType::Master, 6, false, 0);
      short_name_ = "ADJ RGB";
    } break;
    case StockFixture::BT_VINTAGE_5CH:
      functions_.emplace_back(FunctionType::White, 0, false, 0);
      functions_.emplace_back(FunctionType::Master, 1, false, 0);
      functions_.emplace_back(FunctionType::Red, 2, false, 1);
      functions_.emplace_back(FunctionType::Green, 3, false, 1);
      functions_.emplace_back(FunctionType::Blue, 4, false, 1);
      class_ = FixtureClass::RingedPar;
      short_name_ = "BTVint";
      break;
    case StockFixture::BT_VINTAGE_6CH:
      functions_.emplace_back(FunctionType::White, 0, false, 0);
      functions_.emplace_back(FunctionType::Master, 1, false, 0);
      functions_.emplace_back(FunctionType::Strobe, 2, false, 0);
      functions_.emplace_back(FunctionType::Red, 3, false, 1);
      functions_.emplace_back(FunctionType::Green, 4, false, 1);
      functions_.emplace_back(FunctionType::Blue, 5, false, 1);
      class_ = FixtureClass::RingedPar;
      short_name_ = "BTVint";
      break;
    case StockFixture::BT_VINTAGE_7CH: {
      functions_.emplace_back(FunctionType::White, 0, false, 0);
      functions_.emplace_back(FunctionType::Master, 1, false, 0);
      functions_.emplace_back(FunctionType::Strobe, 2, false, 0);
      functions_.emplace_back(FunctionType::Red, 3, false, 1);
      functions_.emplace_back(FunctionType::Green, 4, false, 1);
      functions_.emplace_back(FunctionType::Blue, 5, false, 1);
      FixtureTypeFunction &macro =
          functions_.emplace_back(FunctionType::ColorMacro, 6, false, 1);
      SetBTMacroParameters(macro.GetMacroParameters());
      class_ = FixtureClass::RingedPar;
      short_name_ = "BTVint";
    } break;
    case StockFixture::RGBLight6Ch_16bit:
      functions_.emplace_back(FunctionType::Red, 0, true, 0);
      functions_.emplace_back(FunctionType::Green, 2, true, 0);
      functions_.emplace_back(FunctionType::Blue, 4, true, 0);
      short_name_ = "RGB";
      break;
  }
  UpdateFunctions();
}

void FixtureType::SetRgbAdj6chMacroParameters(MacroParameters &macro) {
  std::vector<MacroParameters::Range> &ranges = macro.GetRanges();
  ranges.emplace_back(0, 8, std::optional<Color>());
  ranges.emplace_back(8, 12, Color::RedC());
  ranges.emplace_back(12, 18, Color::GreenC());
  ranges.emplace_back(18, 24, Color::BlueC());
  ranges.emplace_back(24, 30, Color::Yellow());
  ranges.emplace_back(30, 36, Color::Purple());
  ranges.emplace_back(36, 42, Color::LBlue());
  ranges.emplace_back(42, 48, Color::White());
  ranges.emplace_back(48, 54, Color::GreenYellow());
  ranges.emplace_back(54, 60, Color::PurpleBlue());
  ranges.emplace_back(60, 66, Color(128, 128, 0));
  ranges.emplace_back(66, 72, Color(128, 0, 128));
  ranges.emplace_back(72, 78, Color(0, 128, 128));
  ranges.emplace_back(78, 84, Color(192, 128, 0));
  ranges.emplace_back(84, 90, Color(192, 0, 128));
  ranges.emplace_back(90, 96, Color(192, 128, 128));
  ranges.emplace_back(96, 102, Color(0, 192, 128));
  ranges.emplace_back(102, 108, Color(128, 192, 0));
  ranges.emplace_back(108, 114, Color(128, 192, 128));
  ranges.emplace_back(114, 120, Color(0, 128, 192));
  ranges.emplace_back(120, 126, Color(128, 0, 192));
  ranges.emplace_back(126, 132, Color(128, 128, 192));
  ranges.emplace_back(132, 138, Color(96, 144, 0));
  ranges.emplace_back(138, 144, Color(96, 0, 144));
  ranges.emplace_back(144, 150, Color(96, 144, 96));
  ranges.emplace_back(150, 156, Color(128, 144, 144));
  ranges.emplace_back(156, 162, Color(0, 96, 144));
  ranges.emplace_back(162, 168, Color(144, 96, 144));
  ranges.emplace_back(168, 174, Color(0, 144, 96));
  ranges.emplace_back(174, 180, Color(144, 0, 96));
  ranges.emplace_back(180, 186, Color(144, 144, 96));
  ranges.emplace_back(186, 192, Color(44, 180, 0));
  ranges.emplace_back(192, 198, Color(44, 0, 180));
  ranges.emplace_back(198, 204, Color(44, 180, 180));
  ranges.emplace_back(204, 210, Color(180, 44, 0));
  ranges.emplace_back(210, 216, Color(0, 44, 180));
  ranges.emplace_back(216, 222, Color(180, 44, 180));
  ranges.emplace_back(222, 228, Color(0, 180, 44));
  ranges.emplace_back(228, 234, Color(180, 0, 44));
  ranges.emplace_back(234, 240, Color(0, 180, 44));
  ranges.emplace_back(240, 246, Color(20, 96, 0));
  ranges.emplace_back(246, 252, Color(20, 0, 96));
  ranges.emplace_back(252, 256, Color(20, 96, 96));
}

void FixtureType::SetH2OMacroParameters(MacroParameters &macro) {
  std::vector<MacroParameters::Range> &ranges = macro.GetRanges();
  ranges.emplace_back(0, 10, Color::White());
  ranges.emplace_back(10, 21, Color::WhiteOrange());
  ranges.emplace_back(21, 32, Color::Orange());
  ranges.emplace_back(32, 43, Color::OrangeGreen());
  ranges.emplace_back(43, 54, Color::GreenC());
  ranges.emplace_back(54, 65, Color::GreenBlue());
  ranges.emplace_back(65, 76, Color::BlueC());
  ranges.emplace_back(76, 87, Color::BlueYellow());
  ranges.emplace_back(87, 98, Color::Yellow());
  ranges.emplace_back(98, 109, Color::YellowPurple());
  ranges.emplace_back(109, 120, Color::Purple());
  ranges.emplace_back(120, 127, Color::PurpleWhite());
  ranges.emplace_back(128, 256, Color::White());
}

void FixtureType::SetBTMacroParameters(MacroParameters &macro) {
  std::vector<MacroParameters::Range> &ranges = macro.GetRanges();
  ranges.emplace_back(0, 8, std::optional<Color>());
  ranges.emplace_back(0, 28, Color::RedC());
  ranges.emplace_back(0, 48, Color::Orange());
  ranges.emplace_back(0, 68, Color::Yellow());
  ranges.emplace_back(0, 88, Color::GreenC());
  ranges.emplace_back(0, 98, Color::Cyan());
  ranges.emplace_back(0, 108, Color::BlueC());
  ranges.emplace_back(0, 118, Color::Purple());
  ranges.emplace_back(0, 256, Color::White());
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
  std::optional<Color> macro_color;
  for (size_t i = 0; i != functions_.size(); ++i) {
    if (functions_[i].Shape() == shapeIndex) {
      const unsigned channel_value = fixture.Functions()[i]->GetValue(snapshot);
      const FunctionType type = functions_[i].Type();
      if (type == FunctionType::Master) {
        master = channel_value;
      } else if (type == FunctionType::ColorMacro) {
        macro_color =
            functions_[i].GetMacroParameters().GetColor(channel_value);
      } else {
        const Color c = GetFunctionColor(functions_[i].Type()) * channel_value;
        red += c.Red();
        green += c.Green();
        blue += c.Blue();
      }
    }
  }
  if (macro_color) {
    red = macro_color->Red();
    green = macro_color->Green();
    blue = macro_color->Blue();
    return Color(red * master / 256, green * master / 256, blue * master / 256);
  } else {
    return Color(red * master / scaling_value_, green * master / scaling_value_,
                 blue * master / scaling_value_);
  }
}

int FixtureType::GetRotationSpeed(const Fixture &fixture,
                                  const ValueSnapshot &snapshot,
                                  size_t shape_index) const {
  for (size_t i = 0; i != functions_.size(); ++i) {
    if (functions_[i].Shape() == shape_index &&
        functions_[i].Type() == FunctionType::Rotation) {
      const unsigned channel_value = fixture.Functions()[i]->GetValue(snapshot);
      return functions_[i].GetRotationParameters().GetSpeed(channel_value);
    }
  }
  return 0;
}

}  // namespace glight::theatre
