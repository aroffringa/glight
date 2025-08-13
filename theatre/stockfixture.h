#ifndef THEATRE_STOCK_FIXTURE_H_
#define THEATRE_STOCK_FIXTURE_H_

#include <string_view>
#include <vector>

namespace glight::theatre {

enum class StockFixture {
  Light1Ch,
  Rgb3Ch,
  Rgb4Ch,
  Rgba4Ch,
  Rgba5Ch,
  Rgbw4Ch,
  RgbUv4Ch,
  Rgbaw5Ch,
  RgbawUv6Ch,
  Rgbl4Ch,
  Uv3Ch,
  H2ODmxPro,
  AyraTDCSunrise,
  AdjStarBurst,
  RGB_ADJ_6CH,
  RGB_ADJ_7CH,
  BT_VINTAGE_5CH,
  BT_VINTAGE_6CH,
  BT_VINTAGE_7CH,
  CWWW2Ch,
  CWWW4Ch,
  CWWWA3Ch,
  RGBLight6Ch_16bit,
  ZoomLight,
  MovingHead,
  ZoomingMovingHead
};

inline constexpr std::string_view ToString(StockFixture fixtureClass) {
  switch (fixtureClass) {
    case StockFixture::Light1Ch:
      return "Light (1ch)";
    case StockFixture::Rgb3Ch:
      return "RGB light (3ch)";
    case StockFixture::Rgb4Ch:
      return "RGB light (4ch)";
    case StockFixture::Rgba4Ch:
      return "RGBA light (4ch)";
    case StockFixture::Rgba5Ch:
      return "RGBA light (5ch)";
    case StockFixture::Rgbw4Ch:
      return "RGBW light (4ch)";
    case StockFixture::RgbUv4Ch:
      return "RGBUV light (4ch)";
    case StockFixture::Rgbaw5Ch:
      return "RGBAW light (5ch)";
    case StockFixture::RgbawUv6Ch:
      return "RGBAW+UV light (6ch)";
    case StockFixture::Rgbl4Ch:
      return "RGBL light (4ch)";
    case StockFixture::CWWW2Ch:
      return "CW/WW light (2ch)";
    case StockFixture::CWWW4Ch:
      return "CW/WW light (4ch)";
    case StockFixture::CWWWA3Ch:
      return "CW/WW/A light (3ch)";
    case StockFixture::Uv3Ch:
      return "UV light (3ch)";
    case StockFixture::H2ODmxPro:
      return "H2O DMX Pro";
    case StockFixture::AdjStarBurst:
      return "ADJ Starburst";
    case StockFixture::AyraTDCSunrise:
      return "Ayra TDC Sunrise";
    case StockFixture::RGB_ADJ_6CH:
      return "RGB ADJ (6ch)";
    case StockFixture::RGB_ADJ_7CH:
      return "RGB ADJ (7ch)";
    case StockFixture::BT_VINTAGE_5CH:
      return "Briteq Vintage (5ch)";
    case StockFixture::BT_VINTAGE_6CH:
      return "Briteq Vintage (6ch)";
    case StockFixture::BT_VINTAGE_7CH:
      return "Briteq Vintage (7ch)";
    case StockFixture::RGBLight6Ch_16bit:
      return "RGB light (6ch, 16 bit)";
    case StockFixture::ZoomLight:
      return "Zoom light (RGB)";
    case StockFixture::MovingHead:
      return "Moving head (RGB)";
    case StockFixture::ZoomingMovingHead:
      return "Zooming moving head (RGB)";
  }
  return "Unknown fixture class";
}

inline std::vector<StockFixture> GetStockFixtureList() {
  using SF = StockFixture;
  return std::vector<SF>{SF::Light1Ch,
                         SF::Rgb3Ch,
                         SF::Rgb4Ch,
                         SF::Rgba4Ch,
                         SF::Rgba5Ch,
                         SF::Rgbw4Ch,
                         SF::RgbUv4Ch,
                         SF::Rgbaw5Ch,
                         SF::RgbawUv6Ch,
                         SF::Rgbl4Ch,
                         SF::CWWW2Ch,
                         SF::CWWW4Ch,
                         SF::CWWWA3Ch,
                         SF::Uv3Ch,
                         SF::H2ODmxPro,
                         SF::AdjStarBurst,
                         SF::AyraTDCSunrise,
                         SF::RGB_ADJ_6CH,
                         SF::RGB_ADJ_7CH,
                         SF::BT_VINTAGE_5CH,
                         SF::BT_VINTAGE_6CH,
                         SF::BT_VINTAGE_7CH,
                         SF::RGBLight6Ch_16bit,
                         SF::ZoomLight,
                         SF::MovingHead,
                         SF::ZoomingMovingHead};
}

}  // namespace glight::theatre

#endif
