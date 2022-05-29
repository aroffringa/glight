#ifndef FIXTURE_TYPE_H_
#define FIXTURE_TYPE_H_

#include <map>
#include <string>
#include <variant>
#include <vector>

#include "color.h"
#include "folderobject.h"
#include "functiontype.h"
#include "valuesnapshot.h"

class Fixture;

struct RotationParameters {
  struct Range {
    unsigned input_min;
    unsigned input_max;
    unsigned speed_min;
    unsigned speed_max;
  };
  std::vector<Range> ranges;
};

class FixtureTypeFunction {
 public:
  FixtureTypeFunction(size_t dmx_offset, FunctionType type, bool is_16_bit,
                      unsigned shape)
      : dmx_offset_(dmx_offset),
        type_(type),
        is_16_bit_(is_16_bit),
        shape_(shape) {}

  size_t DmxOffset() const { return dmx_offset_; }
  FunctionType Type() const { return type_; }
  void SetType(FunctionType type) {
    type_ = type;
    if (type_ == FunctionType::Rotation)
      parameters.rotation_parameters = RotationParameters();
  }
  bool Is16Bit() const { return is_16_bit_; }
  unsigned Shape() const { return shape_; }

 private:
  size_t dmx_offset_;
  FunctionType type_;
  bool is_16_bit_;
  unsigned shape_;
  union {
    std::monostate m;
    RotationParameters rotation_parameters;
  } parameters = {std::monostate()};
};

// These currently have a fixed position, because they are written
// as integers into save files... TODO write as name into the file.
enum class StockFixture {
  Light1Ch,
  RGBLight3Ch,
  RGBLight4Ch,
  RGBALight4Ch,
  RGBALight5Ch,
  RGBWLight4Ch,
  RGBUVLight4Ch,
  RGBAWLight5Ch,
  RGBAWUVLight6Ch,
  UVLight3Ch,
  H2ODMXPro,
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
  FixtureType() = default;

  FixtureType(StockFixture stock_fixture);

  FixtureType(const FixtureType &fixtureType) = default;

  static const std::string StockName(StockFixture fixtureClass) {
    switch (fixtureClass) {
      case StockFixture::Light1Ch:
        return "Light (1ch)";
      case StockFixture::RGBLight3Ch:
        return "RGB light (3ch)";
      case StockFixture::RGBLight4Ch:
        return "RGB light (4ch)";
      case StockFixture::RGBALight4Ch:
        return "RGBA light (4ch)";
      case StockFixture::RGBALight5Ch:
        return "RGBA light (5ch)";
      case StockFixture::RGBWLight4Ch:
        return "RGBW light (4ch)";
      case StockFixture::RGBUVLight4Ch:
        return "RGBUV light (4ch)";
      case StockFixture::RGBAWLight5Ch:
        return "RGBAW light (5ch)";
      case StockFixture::RGBAWUVLight6Ch:
        return "RGBAW+UV light (6ch)";
      case StockFixture::CWWW2Ch:
        return "CW/WW light (2ch)";
      case StockFixture::CWWW4Ch:
        return "CW/WW light (4ch)";
      case StockFixture::CWWWA3Ch:
        return "CW/WW/A light (3ch)";
      case StockFixture::UVLight3Ch:
        return "UV light (3ch)";
      case StockFixture::H2ODMXPro:
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
        SF::Light1Ch,       SF::RGBLight3Ch,    SF::RGBLight4Ch,
        SF::RGBALight4Ch,   SF::RGBALight5Ch,   SF::RGBWLight4Ch,
        SF::RGBUVLight4Ch,  SF::RGBAWLight5Ch,  SF::RGBAWUVLight6Ch,
        SF::CWWW2Ch,        SF::CWWW4Ch,        SF::CWWWA3Ch,
        SF::UVLight3Ch,     SF::H2ODMXPro,      SF::AyraTDCSunrise,
        SF::RGB_ADJ_6CH,    SF::RGB_ADJ_7CH,    SF::BT_VINTAGE_5CH,
        SF::BT_VINTAGE_6CH, SF::BT_VINTAGE_7CH, SF::RGBLight6Ch_16bit};
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
    throw std::runtime_error("Class not found: " + name);
  }

  Color GetColor(const Fixture &fixture, const ValueSnapshot &snapshot,
                 size_t shapeIndex) const;

  /**
   * Determine the rotation speed of the fixture corresponding with the
   * snapshot. 0 is no rotation, +/- 2^24 is 100 times per second (the max).
   * Positive is clockwise rotation.
   */
  int GetRotationSpeed(const Fixture &fixture,
                       const ValueSnapshot &snapshot) const;

  FixtureClass GetFixtureClass() const { return _class; }

  void SetFixtureClass(FixtureClass new_class) { _class = new_class; }

  bool Is16Bit(size_t functionIndex) const {
    return _functions[functionIndex].Is16Bit();
  }

  size_t ShapeCount() const {
    switch (_class) {
      case FixtureClass::Par:
        return 1;
      case FixtureClass::RingedPar:
        return 2;
    }
    return 0;
  }

  const std::vector<FixtureTypeFunction> &Functions() const {
    return _functions;
  }

  void SetFunctions(const std::vector<FixtureTypeFunction> &functions) {
    _functions = functions;
    UpdateFunctions();
  }
  void SetFunctions(std::vector<FixtureTypeFunction> &&functions) {
    _functions = std::move(functions);
    UpdateFunctions();
  }

 private:
  void UpdateFunctions();

  static Color rgbAdj6chColor(const Fixture &fixture,
                              const ValueSnapshot &snapshot);

  FixtureClass _class = FixtureClass::Par;
  std::vector<FixtureTypeFunction> _functions;
  unsigned scaling_value_;
};

#endif
