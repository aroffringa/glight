#ifndef FIXTURETYPE_H
#define FIXTURETYPE_H

#include <string>
#include <vector>

#include "color.h"
#include "folderobject.h"
#include "functiontype.h"
#include "valuesnapshot.h"

class Fixture;

struct FixtureTypeFunction {
  FixtureTypeFunction(size_t dmxOffset_, FunctionType type_, bool is16Bit_)
      : dmxOffset(dmxOffset_), type(type_), is16Bit(is16Bit_) {}

  size_t dmxOffset;
  FunctionType type;
  bool is16Bit;
};

// These currently have a fixed position, because they are written
// as integers into save files... TODO write as name into the file.
enum class FixtureClass {
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

/**
 *  @author Andre Offringa
 */
class FixtureType : public FolderObject {
 public:
  FixtureType(FixtureClass fixtureClass);

  FixtureType(const FixtureType &fixtureType)
      : FolderObject(fixtureType),
        _class(fixtureType._class),
        _functions(fixtureType._functions) {}

  static const std::string ClassName(FixtureClass fixtureClass) {
    switch (fixtureClass) {
      case FixtureClass::Light1Ch:
        return "Light (1ch)";
      case FixtureClass::RGBLight3Ch:
        return "RGB light (3ch)";
      case FixtureClass::RGBLight4Ch:
        return "RGB light (4ch)";
      case FixtureClass::RGBALight4Ch:
        return "RGBA light (4ch)";
      case FixtureClass::RGBALight5Ch:
        return "RGBA light (5ch)";
      case FixtureClass::RGBWLight4Ch:
        return "RGBW light (4ch)";
      case FixtureClass::RGBUVLight4Ch:
        return "RGBUV light (4ch)";
      case FixtureClass::RGBAWLight5Ch:
        return "RGBAW light (5ch)";
      case FixtureClass::RGBAWUVLight6Ch:
        return "RGBAW+UV light (6ch)";
      case FixtureClass::CWWW2Ch:
        return "CW/WW light (2ch)";
      case FixtureClass::CWWW4Ch:
        return "CW/WW light (4ch)";
      case FixtureClass::CWWWA3Ch:
        return "CW/WW/A light (3ch)";
      case FixtureClass::UVLight3Ch:
        return "UV light (3ch)";
      case FixtureClass::H2ODMXPro:
        return "H2O DMX Pro";
      case FixtureClass::AyraTDCSunrise:
        return "Ayra TDC Sunrise";
      case FixtureClass::RGB_ADJ_6CH:
        return "RGB ADJ (6ch)";
      case FixtureClass::RGB_ADJ_7CH:
        return "RGB ADJ (7ch)";
      case FixtureClass::BT_VINTAGE_5CH:
        return "Briteq Vintage (5ch)";
      case FixtureClass::BT_VINTAGE_6CH:
        return "Briteq Vintage (6ch)";
      case FixtureClass::BT_VINTAGE_7CH:
        return "Briteq Vintage (7ch)";
      case FixtureClass::RGBLight6Ch_16bit:
        return "RGB light (6ch, 16 bit)";
    }
    return "Unknown fixture class";
  }

  static std::vector<enum FixtureClass> GetClassList() {
    using FC = FixtureClass;
    return std::vector<enum FixtureClass>{
        FC::Light1Ch,       FC::RGBLight3Ch,    FC::RGBLight4Ch,
        FC::RGBALight4Ch,   FC::RGBALight5Ch,   FC::RGBWLight4Ch,
        FC::RGBUVLight4Ch,  FC::RGBAWLight5Ch,  FC::RGBAWUVLight6Ch,
        FC::CWWW2Ch,        FC::CWWW4Ch,        FC::CWWWA3Ch,
        FC::UVLight3Ch,     FC::H2ODMXPro,      FC::AyraTDCSunrise,
        FC::RGB_ADJ_6CH,    FC::RGB_ADJ_7CH,    FC::BT_VINTAGE_5CH,
        FC::BT_VINTAGE_6CH, FC::BT_VINTAGE_7CH, FC::RGBLight6Ch_16bit};
  }

  static FixtureClass NameToClass(const std::string &name) {
    const std::vector<enum FixtureClass> list = GetClassList();
    for (const enum FixtureClass &cl : list)
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

  enum FixtureClass GetFixtureClass() const { return _class; }

  void SetFixtureClass(enum FixtureClass new_class) { _class = new_class; }

  bool Is16Bit([[maybe_unused]] size_t functionIndex) const {
    switch (_class) {
      case FixtureClass::RGBLight6Ch_16bit:
        return true;
      default:
        return false;
    }
  }
  size_t ShapeCount() const {
    using FC = FixtureClass;
    switch (_class) {
      case FC::Light1Ch:
      case FC::RGBLight3Ch:
      case FC::RGBLight4Ch:
      case FC::RGBALight4Ch:
      case FC::RGBALight5Ch:
      case FC::RGBWLight4Ch:
      case FC::RGBUVLight4Ch:
      case FC::RGBAWLight5Ch:
      case FC::RGBAWUVLight6Ch:
      case FC::CWWW2Ch:
      case FC::CWWW4Ch:
      case FC::CWWWA3Ch:
      case FC::UVLight3Ch:
      case FC::H2ODMXPro:
      case FC::AyraTDCSunrise:
      case FC::RGB_ADJ_6CH:
      case FC::RGB_ADJ_7CH:
      case FC::RGBLight6Ch_16bit:
        return 1;
      case FC::BT_VINTAGE_5CH:
      case FC::BT_VINTAGE_6CH:
      case FC::BT_VINTAGE_7CH:
        return 2;
    }
    return 0;
  }

  const std::vector<FixtureTypeFunction> &Functions() const {
    return _functions;
  }

  void SetFunctions(const std::vector<FixtureTypeFunction> &functions) {
    _functions = functions;
  }
  void SetFunctions(std::vector<FixtureTypeFunction> &&functions) {
    _functions = std::move(functions);
  }

 private:
  static Color rgbAdj6chColor(const Fixture &fixture,
                              const ValueSnapshot &snapshot);

  enum FixtureClass _class;
  std::vector<FixtureTypeFunction> _functions;
};

#endif
