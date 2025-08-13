#include "fixturetype.h"

#include "system/optionalnumber.h"

using glight::system::OptionalNumber;

namespace glight::theatre {

FixtureType::FixtureType(const std::string& name) : FolderObject(name) {}

FixtureType::FixtureType(StockFixture stock_fixture)
    : FolderObject(ToString(stock_fixture)) {
  constexpr OptionalNumber<size_t> empty_channel;
  std::vector<FixtureModeFunction> functions;
  switch (stock_fixture) {
    case StockFixture::Light1Ch:
      functions.emplace_back(FunctionType::White, 0, empty_channel, 0);
      data_.short_name_ = "Light";
      break;
    case StockFixture::Rgb3Ch:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      data_.short_name_ = "RGB";
      break;
    case StockFixture::Rgb4Ch:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Master, 3, empty_channel, 0);
      data_.short_name_ = "RGB";
      break;
    case StockFixture::Rgba4Ch:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Amber, 3, empty_channel, 0);
      data_.short_name_ = "RGBA";
      break;
    case StockFixture::Rgba5Ch:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Amber, 3, empty_channel, 0);
      functions.emplace_back(FunctionType::Master, 4, empty_channel, 0);
      data_.short_name_ = "RGBA";
      break;
    case StockFixture::Rgbw4Ch:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::White, 3, empty_channel, 0);
      data_.short_name_ = "RGBW";
      break;
    case StockFixture::RgbUv4Ch:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::UV, 3, empty_channel, 0);
      data_.short_name_ = "RGBUV";
      break;
    case StockFixture::Rgbaw5Ch:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Amber, 3, empty_channel, 0);
      functions.emplace_back(FunctionType::White, 4, empty_channel, 0);
      data_.short_name_ = "RGBAW";
      break;
    case StockFixture::RgbawUv6Ch:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Amber, 3, empty_channel, 0);
      functions.emplace_back(FunctionType::White, 4, empty_channel, 0);
      functions.emplace_back(FunctionType::UV, 5, empty_channel, 0);
      data_.short_name_ = "RGBAWUV";
      break;
    case StockFixture::Rgbl4Ch:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Lime, 3, empty_channel, 0);
      data_.short_name_ = "RGBL";
      break;
    case StockFixture::CWWW2Ch:
      functions.emplace_back(FunctionType::ColdWhite, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::WarmWhite, 1, empty_channel, 0);
      data_.short_name_ = "CW/WW";
      break;
    case StockFixture::CWWW4Ch:
      functions.emplace_back(FunctionType::Master, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::ColdWhite, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::WarmWhite, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Strobe, 3, empty_channel, 0);
      data_.short_name_ = "CW/WW";
      break;
    case StockFixture::CWWWA3Ch:
      functions.emplace_back(FunctionType::ColdWhite, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::WarmWhite, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Amber, 2, empty_channel, 0);
      data_.short_name_ = "CW/WW/A";
      break;
    case StockFixture::Uv3Ch:
      functions.emplace_back(FunctionType::UV, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Strobe, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Pulse, 2, empty_channel, 0);
      data_.short_name_ = "UV";
      break;
    case StockFixture::H2ODmxPro: {
      functions.emplace_back(FunctionType::Master, 0, empty_channel, 0);
      FixtureModeFunction &rotation = functions.emplace_back(
          FunctionType::RotationSpeed, 1, empty_channel, 0);
      std::vector<RotationSpeedParameters::Range> &ranges =
          rotation.GetRotationParameters().GetRanges();
      constexpr int max_speed = (1 << 24) / 100;  // 1 times per second
      ranges.emplace_back(9, 121, 0, max_speed);
      ranges.emplace_back(134, 256, 0, -max_speed);
      FixtureModeFunction &macro = functions.emplace_back(
          FunctionType::ColorMacro, 2, empty_channel, 0);
      SetH2OMacroParameters(macro.GetColorRangeParameters());
      data_.short_name_ = "H2O";
    } break;
    case StockFixture::AdjStarBurst: {
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::White, 3, empty_channel, 0);
      functions.emplace_back(FunctionType::Amber, 4, empty_channel, 0);
      functions.emplace_back(FunctionType::UV, 5, empty_channel, 0);
      functions.emplace_back(FunctionType::Strobe, 6, empty_channel, 0);
      functions.emplace_back(FunctionType::Master, 7, empty_channel, 0);
      FixtureModeFunction &rotation = functions.emplace_back(
          FunctionType::RotationSpeed, 8, empty_channel, 0);
      functions.emplace_back(FunctionType::ColorMacro, 9, empty_channel, 0);
      functions.emplace_back(FunctionType::Effect, 10, empty_channel, 0);
      functions.emplace_back(FunctionType::Effect, 11, empty_channel, 0);
      std::vector<RotationSpeedParameters::Range> &ranges =
          rotation.GetRotationParameters().GetRanges();
      constexpr int max_speed = (1 << 24) / 100;  // 1 times per second
      ranges.emplace_back(31, 141, max_speed, 0);
      ranges.emplace_back(146, 256, 0, -max_speed);
      data_.short_name_ = "Starbrst";
      data_.min_beam_angle_ = 2.0 * M_PI;
      data_.brightness_ = 3.0;
    } break;
    case StockFixture::AyraTDCSunrise: {
      functions.emplace_back(FunctionType::Master, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Red, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 3, empty_channel, 0);
      functions.emplace_back(FunctionType::Strobe, 4, empty_channel, 0);
      FixtureModeFunction &rotation = functions.emplace_back(
          FunctionType::RotationSpeed, 5, empty_channel, 0);
      constexpr int max_speed = (1 << 24) / 100;  // 1 times per second
      std::vector<RotationSpeedParameters::Range> &ranges =
          rotation.GetRotationParameters().GetRanges();
      // [ 5 - 128 ) is static positioning
      ranges.emplace_back(128, 256, -max_speed, max_speed);
      functions.emplace_back(FunctionType::ColorMacro, 6, empty_channel, 0);
      data_.short_name_ = "TDC Sunr";
      data_.min_beam_angle_ = 2.0 * M_PI;
      data_.brightness_ = 1.0;
    } break;
    case StockFixture::RGB_ADJ_6CH: {
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      FixtureModeFunction &macro = functions.emplace_back(
          FunctionType::ColorMacro, 3, empty_channel, 0);
      SetRgbAdj6chMacroParameters(macro.GetColorRangeParameters());
      functions.emplace_back(FunctionType::Strobe, 4, empty_channel, 0);
      functions.emplace_back(FunctionType::Pulse, 5, empty_channel, 0);
      data_.short_name_ = "ADJ RGB";
    } break;
    case StockFixture::RGB_ADJ_7CH: {
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      FixtureModeFunction &macro = functions.emplace_back(
          FunctionType::ColorMacro, 3, empty_channel, 0);
      SetRgbAdj6chMacroParameters(macro.GetColorRangeParameters());
      functions.emplace_back(FunctionType::Strobe, 4, empty_channel, 0);
      functions.emplace_back(FunctionType::Pulse, 5, empty_channel, 0);
      functions.emplace_back(FunctionType::Master, 6, empty_channel, 0);
      data_.short_name_ = "ADJ RGB";
    } break;
    case StockFixture::BT_VINTAGE_5CH:
      functions.emplace_back(FunctionType::White, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Master, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Red, 2, empty_channel, 1);
      functions.emplace_back(FunctionType::Green, 3, empty_channel, 1);
      functions.emplace_back(FunctionType::Blue, 4, empty_channel, 1);
      data_.class_ = FixtureClass::RingedPar;
      data_.short_name_ = "BTVint";
      break;
    case StockFixture::BT_VINTAGE_6CH:
      functions.emplace_back(FunctionType::White, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Master, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Strobe, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Red, 3, empty_channel, 1);
      functions.emplace_back(FunctionType::Green, 4, empty_channel, 1);
      functions.emplace_back(FunctionType::Blue, 5, empty_channel, 1);
      data_.class_ = FixtureClass::RingedPar;
      data_.short_name_ = "BTVint";
      break;
    case StockFixture::BT_VINTAGE_7CH: {
      functions.emplace_back(FunctionType::White, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Master, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Strobe, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Red, 3, empty_channel, 1);
      functions.emplace_back(FunctionType::Green, 4, empty_channel, 1);
      functions.emplace_back(FunctionType::Blue, 5, empty_channel, 1);
      FixtureModeFunction &macro = functions.emplace_back(
          FunctionType::ColorMacro, 6, empty_channel, 1);
      SetBTMacroParameters(macro.GetColorRangeParameters());
      data_.class_ = FixtureClass::RingedPar;
      data_.short_name_ = "BTVint";
    } break;
    case StockFixture::RGBLight6Ch_16bit:
      functions.emplace_back(FunctionType::Red, 0, OptionalNumber<size_t>(1),
                              0);
      functions.emplace_back(FunctionType::Green, 2, OptionalNumber<size_t>(3),
                              0);
      functions.emplace_back(FunctionType::Blue, 4, OptionalNumber<size_t>(5),
                              0);
      data_.short_name_ = "RGB";
      break;
    case StockFixture::ZoomLight:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Zoom, 3, empty_channel, 0);
      data_.min_beam_angle_ = 10.0 * M_PI / 180.0;
      data_.max_beam_angle_ = 50.0 * M_PI / 180.0;
      data_.short_name_ = "Zoom";
      break;
    case StockFixture::MovingHead:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Pan, 3, empty_channel, 0);
      functions.emplace_back(FunctionType::Tilt, 4, empty_channel, 0);
      data_.min_pan_ = 0.0;
      data_.max_pan_ = 2.0 * M_PI;
      data_.min_tilt_ = -0.25 * M_PI;
      data_.max_tilt_ = 0.5 * M_PI;
      data_.short_name_ = "Move";
      break;
    case StockFixture::ZoomingMovingHead:
      functions.emplace_back(FunctionType::Red, 0, empty_channel, 0);
      functions.emplace_back(FunctionType::Green, 1, empty_channel, 0);
      functions.emplace_back(FunctionType::Blue, 2, empty_channel, 0);
      functions.emplace_back(FunctionType::Pan, 3, empty_channel, 0);
      functions.emplace_back(FunctionType::Tilt, 4, empty_channel, 0);
      data_.min_pan_ = 0.0;
      data_.max_pan_ = 2.0 * M_PI;
      data_.min_tilt_ = -0.25 * M_PI;
      data_.max_tilt_ = 0.5 * M_PI;
      functions.emplace_back(FunctionType::Zoom, 5, empty_channel, 0);
      data_.min_beam_angle_ = 10.0 * M_PI / 180.0;
      data_.max_beam_angle_ = 50.0 * M_PI / 180.0;
      data_.short_name_ = "Move+Zoom";
      break;
  }
  data_.max_power_ = 0;
  for (FixtureModeFunction &function : functions) {
    if (IsColor(function.Type())) {
      function.SetPower(10);
    }
    data_.max_power_ += function.Power();
  }
  data_.idle_power_ = 3;

  FixtureMode& mode = modes_.emplace_back(*this);
  mode.SetName("Default");
  mode.SetFunctions(functions);
}

void FixtureType::SetRgbAdj6chMacroParameters(ColorRangeParameters &macro) {
  std::vector<ColorRangeParameters::Range> &ranges = macro.GetRanges();
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

void FixtureType::SetH2OMacroParameters(ColorRangeParameters &macro) {
  std::vector<ColorRangeParameters::Range> &ranges = macro.GetRanges();
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

void FixtureType::SetBTMacroParameters(ColorRangeParameters &macro) {
  std::vector<ColorRangeParameters::Range> &ranges = macro.GetRanges();
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

} // namespace glight::theatre
