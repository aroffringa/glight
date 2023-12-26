#ifndef THEATRE_FIXTURE_TYPE_H_
#define THEATRE_FIXTURE_TYPE_H_

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "color.h"
#include "folderobject.h"
#include "fixturetypefunction.h"
#include "valuesnapshot.h"

namespace glight::theatre {

class Fixture;

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
  RGB_ADJ_6CH,
  RGB_ADJ_7CH,
  BT_VINTAGE_5CH,
  BT_VINTAGE_6CH,
  BT_VINTAGE_7CH,
  CWWW2Ch,
  CWWW4Ch,
  CWWWA3Ch,
  RGBLight6Ch_16bit
};

enum class FixtureClass {
  /**
   * Single light like a par or spot with one or multiple colours.
   */
  Par,
  /**
   * Par with two controlable lights, of which one forms a ring around the
   * other.
   */
  RingedPar
};

/**
 *  @author Andre Offringa
 */
class FixtureType : public FolderObject {
 public:
  FixtureType() : FolderObject() { short_name_ = Name(); }

  FixtureType(StockFixture stock_fixture);

  FixtureType(const FixtureType &fixtureType) = default;

  static const std::string StockName(StockFixture fixtureClass) {
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
    }
    return "Unknown fixture class";
  }

  static const std::string ClassName(FixtureClass fixtureClass) {
    switch (fixtureClass) {
      case FixtureClass::Par:
        return "Par / spot";
      case FixtureClass::RingedPar:
        return "Ringed par";
    }
    return {};
  }

  static std::vector<StockFixture> GetStockList() {
    using SF = StockFixture;
    return std::vector<SF>{
        SF::Light1Ch,         SF::Rgb3Ch,         SF::Rgb4Ch,
        SF::Rgba4Ch,          SF::Rgba5Ch,        SF::Rgbw4Ch,
        SF::RgbUv4Ch,         SF::Rgbaw5Ch,       SF::RgbawUv6Ch,
        SF::Rgbl4Ch,          SF::CWWW2Ch,        SF::CWWW4Ch,
        SF::CWWWA3Ch,         SF::Uv3Ch,          SF::H2ODmxPro,
        SF::AyraTDCSunrise,   SF::RGB_ADJ_6CH,    SF::RGB_ADJ_7CH,
        SF::BT_VINTAGE_5CH,   SF::BT_VINTAGE_6CH, SF::BT_VINTAGE_7CH,
        SF::RGBLight6Ch_16bit};
  }

  static std::vector<FixtureClass> GetClassList() {
    using FC = FixtureClass;
    return std::vector<FC>{FC::Par, FC::RingedPar};
  }

  static std::map<std::string, FixtureType> GetStockTypes() {
    const std::vector<StockFixture> list = GetStockList();
    std::map<std::string, FixtureType> stockTypes;
    for (StockFixture fc : list) {
      stockTypes.emplace(StockName(fc), FixtureType(fc));
    }
    return stockTypes;
  }

  static FixtureClass NameToClass(const std::string &name) {
    const std::vector<FixtureClass> list = GetClassList();
    for (const FixtureClass &cl : list)
      if (ClassName(cl) == name) return cl;
    throw std::runtime_error("Fixture class not found: " + name);
  }

  Color GetColor(const Fixture &fixture, const ValueSnapshot &snapshot,
                 size_t shape_index) const;

  /**
   * Determine the rotation speed of the fixture corresponding with the
   * snapshot. 0 is no rotation, +/- 2^24 is 100 times per second (the max).
   * Positive is clockwise rotation.
   */
  int GetRotationSpeed(const Fixture &fixture, const ValueSnapshot &snapshot,
                       size_t shape_index) const;

  double GetPan(const Fixture &fixture, const ValueSnapshot &snapshot,
                size_t shape_index) const;

  double GetTilt(const Fixture &fixture, const ValueSnapshot &snapshot,
                 size_t shape_index) const;

  double GetZoom(const Fixture &fixture, const ValueSnapshot &snapshot,
                 size_t shape_index) const;

  FixtureClass GetFixtureClass() const { return class_; }

  void SetFixtureClass(FixtureClass new_class) { class_ = new_class; }

  size_t ShapeCount() const {
    switch (class_) {
      case FixtureClass::Par:
        return 1;
      case FixtureClass::RingedPar:
        return 2;
    }
    return 0;
  }

  const std::vector<FixtureTypeFunction> &Functions() const {
    return functions_;
  }

  void SetFunctions(const std::vector<FixtureTypeFunction> &functions) {
    functions_ = functions;
    UpdateFunctions();
  }
  void SetFunctions(std::vector<FixtureTypeFunction> &&functions) {
    functions_ = std::move(functions);
    UpdateFunctions();
  }
  unsigned ColorScalingValue() const { return scaling_value_; }

  const std::string &ShortName() const { return short_name_; }
  void SetShortName(const std::string &short_name) { short_name_ = short_name; }

  /**
   * For a non-zoomable fixture, the static full-width half-maximum angle
   * of the beam in radians. If the fixture can zoom, it returns the
   * smallest angle that the beam can make. If the fixture has no beam, it will
   * be zero.
   */
  double MinBeamAngle() const { return min_beam_angle_; }
  void SetMinBeamAngle(double min_beam_angle) {
    min_beam_angle_ = min_beam_angle;
  }

  double MaxBeamAngle() const { return max_beam_angle_; }
  void SetMaxBeamAngle(double max_beam_angle) {
    max_beam_angle_ = max_beam_angle;
  }

  /**
   * Pan is the horizonal / primary axis rotation of a beamed device (e.g.
   * moving head), in radians.
   */
  double MinPan() const { return min_pan_; }
  void SetMinPan(double min_pan) { min_pan_ = min_pan; }

  double MaxPan() const { return max_pan_; }
  void SetMaxPan(double max_pan) { max_pan_ = max_pan; }

  /**
   * Tilt is the vertical / 2nd axis rotation of a beamed device (e.g. moving
   * head), in radians.
   */
  double MinTilt() const { return min_tilt_; }
  void SetMinTilt(double min_tilt) { min_tilt_ = min_tilt; }

  double MaxTilt() const { return max_tilt_; }
  void SetMaxTilt(double max_tilt) { max_tilt_ = max_tilt; }

  bool CanZoom() const { return min_beam_angle_ != max_beam_angle_; }
  bool CanBeamRotate() const { return min_pan_ != max_pan_; }
  bool CanBeamTilt() const { return min_tilt_ != max_tilt_; }

  /**
   * Distance in meters that the beam can reach at the static beam angle.
   * For a zoomable fixture, it is the reached distance when using the smallest
   * angle.
   */
  double Brightness() const { return brightness_; }
  void SetBrightness(double brightness) { brightness_ = brightness; }

 private:
  void UpdateFunctions();

  static void SetRgbAdj6chMacroParameters(MacroParameters &macro);
  static void SetH2OMacroParameters(MacroParameters &macro);
  static void SetBTMacroParameters(MacroParameters &macro);

  FixtureClass class_ = FixtureClass::Par;
  std::vector<FixtureTypeFunction> functions_;
  unsigned scaling_value_;
  std::string short_name_;
  double min_beam_angle_ = 30.0 * M_PI / 180.0;
  double max_beam_angle_ = 30.0 * M_PI / 180.0;
  double min_pan_ = 0.0;
  double max_pan_ = 0.0;
  double min_tilt_ = 0.0;
  double max_tilt_ = 0.0;
  double brightness_ = 10.0;
};

inline std::string FunctionSummary(const FixtureType &fixture_type) {
  const std::vector<FixtureTypeFunction> &functions = fixture_type.Functions();
  if (functions.empty())
    return "-";
  else {
    std::ostringstream s;
    s << AbbreviatedFunctionType(functions.front().Type());
    for (size_t i = 1; i != functions.size(); ++i) {
      s << "-" << AbbreviatedFunctionType(functions[i].Type());
    }
    return s.str();
  }
}

}  // namespace glight::theatre

#endif
