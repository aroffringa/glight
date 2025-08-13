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
  for (const FixtureClass& cl : list)
    if (ToString(cl) == name) return cl;
  throw std::runtime_error("Fixture class not found: " + std::string(name));
}

class FixtureType : public FolderObject {
 public:
  FixtureType() = default;

  FixtureType(StockFixture stock_fixture);

  FixtureType(const std::string& name);

  FixtureType(const FixtureType& source)
      : FolderObject(source), data_(source.data_) {
    // The modes have a pointer to the fixture type, so need to
    // be explicitly copied.
    for (const FixtureMode& source_mode : source.Modes()) {
      FixtureMode& new_mode = AddMode();
      new_mode.SetName(source_mode.Name());
      new_mode.SetFunctions(source_mode.Functions());
    }
  }

  FixtureClass GetFixtureClass() const { return data_.class_; }

  void SetFixtureClass(FixtureClass new_class) { data_.class_ = new_class; }

  size_t ShapeCount() const {
    switch (data_.class_) {
      case FixtureClass::Par:
        return 1;
      case FixtureClass::RingedPar:
        return 2;
    }
    return 0;
  }

  FixtureMode& AddMode() { return modes_.emplace_back(*this); }

  std::vector<FixtureMode>& Modes() { return modes_; }

  const std::vector<FixtureMode>& Modes() const { return modes_; }

  size_t ModeIndex(const FixtureMode& mode) const {
    for (size_t index = 0; index != modes_.size(); ++index) {
      if (&mode == &modes_[index]) return index;
    }
    throw std::runtime_error("ModeIndex(): can't find specified mode");
  }

  const std::string& ShortName() const { return data_.short_name_; }
  void SetShortName(const std::string& short_name) {
    data_.short_name_ = short_name;
  }

  /**
   * For a non-zoomable fixture, the static full-width half-maximum angle
   * of the beam in radians. If the fixture can zoom, it returns the
   * smallest angle that the beam can make. If the fixture has no beam, it will
   * be zero.
   */
  double MinBeamAngle() const { return data_.min_beam_angle_; }
  void SetMinBeamAngle(double min_beam_angle) {
    data_.min_beam_angle_ = min_beam_angle;
  }

  double MaxBeamAngle() const { return data_.max_beam_angle_; }
  void SetMaxBeamAngle(double max_beam_angle) {
    data_.max_beam_angle_ = max_beam_angle;
  }

  /**
   * Pan is the horizonal / primary axis rotation of a beamed device (e.g.
   * moving head), in radians.
   */
  double MinPan() const { return data_.min_pan_; }
  void SetMinPan(double min_pan) { data_.min_pan_ = min_pan; }

  double MaxPan() const { return data_.max_pan_; }
  void SetMaxPan(double max_pan) { data_.max_pan_ = max_pan; }

  /**
   * Tilt is the vertical / 2nd axis rotation of a beamed device (e.g. moving
   * head), in radians.
   */
  double MinTilt() const { return data_.min_tilt_; }
  void SetMinTilt(double min_tilt) { data_.min_tilt_ = min_tilt; }

  double MaxTilt() const { return data_.max_tilt_; }
  void SetMaxTilt(double max_tilt) { data_.max_tilt_ = max_tilt; }

  bool CanZoom() const {
    return data_.min_beam_angle_ != data_.max_beam_angle_;
  }
  bool CanBeamRotate() const { return data_.min_pan_ != data_.max_pan_; }
  bool CanBeamTilt() const { return data_.min_tilt_ != data_.max_tilt_; }

  /**
   * Distance in meters that the beam can reach at the static beam angle.
   * For a zoomable fixture, it is the reached distance when using the smallest
   * angle.
   */
  double Brightness() const { return data_.brightness_; }
  void SetBrightness(double brightness) { data_.brightness_ = brightness; }

  /// Maximum power drawn by this fixture, in watts.
  unsigned MaxPower() const { return data_.max_power_; }
  void SetMaxPower(unsigned max_power) { data_.max_power_ = max_power; }

  unsigned IdlePower() const { return data_.idle_power_; }
  void SetIdlePower(unsigned idle_power) { data_.idle_power_ = idle_power; }

 private:
  static void SetRgbAdj6chMacroParameters(ColorRangeParameters& macro);
  static void SetH2OMacroParameters(ColorRangeParameters& macro);
  static void SetBTMacroParameters(ColorRangeParameters& macro);

  std::vector<FixtureMode> modes_;

  struct Data {
    FixtureClass class_ = FixtureClass::Par;
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
  } data_;
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
