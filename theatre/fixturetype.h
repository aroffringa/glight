#ifndef FIXTURETYPE_H
#define FIXTURETYPE_H

#include <string>
#include <vector>

#include "color.h"
#include "folderobject.h"
#include "functiontype.h"
#include "valuesnapshot.h"

/**
 *  @author Andre Offringa
 */
class FixtureType : public FolderObject {
 public:
  // These currently have a fixed position, because they are written
  // as integers into save files... TODO write as name into the file.
  enum FixtureClass {
    Light1Ch,
    RGBLight3Ch,
    RGBLight4Ch,
    RGBALight4Ch,
    RGBALight5Ch,
    RGBWLight4Ch,
    RGBUVLight4Ch,
    UVLight3Ch,
    H2ODMXPro,
    RGB_ADJ_6CH,
    RGB_ADJ_7CH,
    BT_VINTAGE_5CH,
    BT_VINTAGE_6CH,
    BT_VINTAGE_7CH,
    RGBAWUVLight6Ch,
    CWWW2Ch,
    CWWW4Ch,
    CWWWA3Ch,
    RGBLight6Ch_16bit
  };

  FixtureType(FixtureClass fixtureClass);

  FixtureType(const FixtureType &fixtureType)
      : FolderObject(fixtureType),
        _class(fixtureType._class),
        _functionTypes(fixtureType._functionTypes) {}

  static const std::string ClassName(FixtureClass fixtureClass) {
    switch (fixtureClass) {
      case Light1Ch:
        return "Light (1ch)";
      case RGBLight3Ch:
        return "RGB light (3ch)";
      case RGBLight4Ch:
        return "RGB light (4ch)";
      case RGBALight4Ch:
        return "RGBA light (4ch)";
      case RGBALight5Ch:
        return "RGBA light (5ch)";
      case RGBWLight4Ch:
        return "RGBW light (4ch)";
      case RGBUVLight4Ch:
        return "RGBUV light (4ch)";
      case RGBAWUVLight6Ch:
        return "RGBAW+UV light (6ch)";
      case CWWW2Ch:
        return "CW/WW light (2ch)";
      case CWWW4Ch:
        return "CW/WW light (4ch)";
      case CWWWA3Ch:
        return "CW/WW/A light (3ch)";
      case UVLight3Ch:
        return "UV light (3ch)";
      case H2ODMXPro:
        return "H2O DMX Pro";
      case RGB_ADJ_6CH:
        return "RGB ADJ (6ch)";
      case RGB_ADJ_7CH:
        return "RGB ADJ (7ch)";
      case BT_VINTAGE_5CH:
        return "Briteq Vintage (5ch)";
      case BT_VINTAGE_6CH:
        return "Briteq Vintage (6ch)";
      case BT_VINTAGE_7CH:
        return "Briteq Vintage (7ch)";
      case RGBLight6Ch_16bit:
        return "RGB light (6ch, 16 bit)";
    }
    return "Unknown fixture class";
  }

  static std::vector<enum FixtureClass> GetClassList() {
    return std::vector<enum FixtureClass>{
        Light1Ch,       RGBLight3Ch,    RGBLight4Ch,      RGBALight4Ch,
        RGBALight5Ch,   RGBWLight4Ch,   RGBUVLight4Ch,    RGBAWUVLight6Ch,
        CWWW2Ch,        CWWW4Ch,        CWWWA3Ch,         UVLight3Ch,
        H2ODMXPro,      RGB_ADJ_6CH,    RGB_ADJ_7CH,      BT_VINTAGE_5CH,
        BT_VINTAGE_6CH, BT_VINTAGE_7CH, RGBLight6Ch_16bit};
  }

  static FixtureClass NameToClass(const std::string &name) {
    const std::vector<enum FixtureClass> list = GetClassList();
    for (const enum FixtureClass &cl : list)
      if (ClassName(cl) == name) return cl;
    throw std::runtime_error("Class not found: " + name);
  }

  Color GetColor(const class Fixture &fixture, const ValueSnapshot &snapshot,
                 size_t shapeIndex) const;

  /**
   * Returns the rotation speed of the fixture.
   * 0 is no rotation, +/- 2^24 is 100 times per second (the max).
   * Positive is clockwise rotation.
   */
  int GetRotationSpeed(const class Fixture &fixture,
                       const ValueSnapshot &snapshot) const;

  enum FixtureClass FixtureClass() const { return _class; }

  bool Is16Bit([[maybe_unused]] size_t functionIndex) const {
    switch (_class) {
      case RGBLight6Ch_16bit:
        return true;
      default:
        return false;
    }
  }
  size_t ShapeCount() const {
    switch (_class) {
      case Light1Ch:
      case RGBLight3Ch:
      case RGBLight4Ch:
      case RGBALight4Ch:
      case RGBALight5Ch:
      case RGBWLight4Ch:
      case RGBUVLight4Ch:
      case RGBAWUVLight6Ch:
      case CWWW2Ch:
      case CWWW4Ch:
      case CWWWA3Ch:
      case UVLight3Ch:
      case H2ODMXPro:
      case RGB_ADJ_6CH:
      case RGB_ADJ_7CH:
      case RGBLight6Ch_16bit:
        return 1;
      case BT_VINTAGE_5CH:
      case BT_VINTAGE_6CH:
      case BT_VINTAGE_7CH:
        return 2;
    }
    return 0;
  }

  const std::vector<FunctionType> FunctionTypes() const {
    return _functionTypes;
  }

 private:
  static Color rgbAdj6chColor(const class Fixture &fixture,
                              const ValueSnapshot &snapshot);

  enum FixtureClass _class;
  std::vector<FunctionType> _functionTypes;
};

#endif
