#ifndef THEATRE_STOCK_FIXTURE_H_
#define THEATRE_STOCK_FIXTURE_H_

#include <string_view>
#include <vector>

namespace glight::theatre {

enum class StockFixture {
  Light,
  Rgb,
  Rgba,
  Rgbw,
  RgbUv,
  Rgbaw,
  RgbawUv,
  Rgbl,
  Uv3Ch,
  H2ODmxPro,
  AyraTDCSunrise,
  AdjStarBurst,
  BtVintage,
  CwWw,
  CwWwA,
  ZoomLight,
  MovingHead,
  ZoomingMovingHead
};

inline constexpr std::string_view ToString(StockFixture fixtureClass) {
  switch (fixtureClass) {
    case StockFixture::Light:
      return "Light";
    case StockFixture::Rgb:
      return "RGB light";
    case StockFixture::Rgba:
      return "RGBA light";
    case StockFixture::Rgbw:
      return "RGBW light";
    case StockFixture::RgbUv:
      return "RGBUV light";
    case StockFixture::Rgbaw:
      return "RGBAW light";
    case StockFixture::RgbawUv:
      return "RGBAW+UV light";
    case StockFixture::Rgbl:
      return "RGBL light";
    case StockFixture::CwWw:
      return "CW/WW light";
    case StockFixture::CwWwA:
      return "CW/WW/A light";
    case StockFixture::Uv3Ch:
      return "UV light";
    case StockFixture::H2ODmxPro:
      return "H2O DMX Pro";
    case StockFixture::AdjStarBurst:
      return "ADJ Starburst";
    case StockFixture::AyraTDCSunrise:
      return "Ayra TDC Sunrise";
    case StockFixture::BtVintage:
      return "Briteq Vintage";
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
  return std::vector<SF>{SF::Light,        SF::Rgb,
                         SF::Rgba,         SF::Rgbw,
                         SF::RgbUv,        SF::Rgbaw,
                         SF::RgbawUv,      SF::Rgbl,
                         SF::CwWw,         SF::CwWwA,
                         SF::Uv3Ch,        SF::H2ODmxPro,
                         SF::AdjStarBurst, SF::AyraTDCSunrise,
                         SF::BtVintage,    SF::ZoomLight,
                         SF::MovingHead,   SF::ZoomingMovingHead};
}

}  // namespace glight::theatre

#endif
