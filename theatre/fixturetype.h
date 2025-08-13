#ifndef THEATRE_FIXTURE_TYPE_H_
#define THEATRE_FIXTURE_TYPE_H_

#include "fixturemode.h"
#include "folderobject.h"

#include <vector>

namespace glight::theatre {

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

inline std::vector<FixtureClass> GetFixtureClassList() {
  using FC = FixtureClass;
  return std::vector<FC>{FC::Par, FC::RingedPar};
}

inline constexpr std::string_view ToString(FixtureClass fixtureClass) {
  switch (fixtureClass) {
    case FixtureClass::Par:
      return "Par / spot";
    case FixtureClass::RingedPar:
      return "Ringed par";
  }
  return {};
}

inline FixtureClass GetFixtureClass(std::string_view name) {
  const std::vector<FixtureClass> list = GetFixtureClassList();
  for (const FixtureClass &cl : list)
    if (ToString(cl) == name) return cl;
  throw std::runtime_error("Fixture class not found: " + std::string(name));
}

class FixtureType : public FolderObject {
public:
  FixtureType() = default;

  FixtureType(StockFixture stock_fixture);

  FixtureType(const std::string& name);

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

  FixtureMode& AddMode() {
    return modes_.emplace_back(*this);
  }

  const std::vector<FixtureMode>& Modes() const {
    return modes_;
  }

  size_t ModeIndex(const FixtureMode& mode) const {
    for(size_t index = 0; index != modes_.size(); ++index) {
      if(&mode == &modes_[index])
        return index;
    }
    throw std::runtime_error("ModeIndex(): can't find specified mode");
  }

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

  /// Maximum power drawn by this fixture, in watts.
  unsigned MaxPower() const { return max_power_; }
  void SetMaxPower(unsigned max_power) { max_power_ = max_power; }

  unsigned IdlePower() const { return idle_power_; }
  void SetIdlePower(unsigned idle_power) { idle_power_ = idle_power; }

private:
  static void SetRgbAdj6chMacroParameters(ColorRangeParameters &macro);
  static void SetH2OMacroParameters(ColorRangeParameters &macro);
  static void SetBTMacroParameters(ColorRangeParameters &macro);

  FixtureClass class_ = FixtureClass::Par;

  std::vector<FixtureMode> modes_;
  std::string short_name_;

  double min_beam_angle_ = 30.0 * M_PI / 180.0;
  double max_beam_angle_ = 30.0 * M_PI / 180.0;
  double min_pan_ = 0.0;
  double max_pan_ = 0.0;
  double min_tilt_ = 0.0;
  double max_tilt_ = 0.0;
  double brightness_ = 10.0;
  unsigned max_power_ = 0;
  unsigned idle_power_ = 0;
};

inline std::map<std::string, FixtureType> GetStockTypes() {
  const std::vector<StockFixture> list = GetStockFixtureList();
  std::map<std::string, FixtureType> stockTypes;
  for (StockFixture fc : list) {
    stockTypes.emplace(ToString(fc), FixtureType(fc));
  }
  return stockTypes;
}

}  // namespace glight::theatre

#endif
