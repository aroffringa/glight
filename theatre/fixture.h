#ifndef THEATRE_FIXTURE_H_
#define THEATRE_FIXTURE_H_

#include <set>

#include "color.h"
#include "coordinate3d.h"
#include "fixturefunction.h"
#include "fixturesymbol.h"
#include "namedobject.h"

namespace glight::theatre {

class FixtureType;
class Coordinate2D;
class Theatre;
class ValueSnapshot;

/**
 * @author Andre Offringa
 */
class Fixture : public NamedObject {
 public:
  Fixture(Theatre &theatre, const FixtureType &type, const std::string &name);
  // Fixture(const Fixture &source, Theatre &theatre);

  static inline constexpr double kDefaultHeight = 5.0;
  static inline constexpr double kDefaultTilt = 0.25 * M_PI;

  const std::vector<std::unique_ptr<FixtureFunction>> &Functions() const {
    return functions_;
  }

  const FixtureType &Type() const { return type_; }

  std::vector<unsigned> GetChannels() const {
    std::vector<unsigned> channels;
    for (const std::unique_ptr<FixtureFunction> &ff : functions_) {
      channels.emplace_back(ff->MainChannel().Channel());
      if (ff->FineChannel())
        channels.emplace_back(ff->FineChannel()->Channel());
    }
    return channels;
  }

  unsigned GetUniverse() const {
    return functions_.front()->MainChannel().Universe();
  }

  void IncChannel();

  void DecChannel();

  void SetChannel(DmxChannel dmx_channel);

  void SetUniverse(unsigned universe);

  DmxChannel GetFirstChannel() const;

  void ClearFunctions() { functions_.clear(); }
  FixtureFunction &AddFunction() {
    functions_.emplace_back(std::make_unique<FixtureFunction>());
    return *functions_.back();
  }
  inline Color GetColor(const ValueSnapshot &snapshot,
                        size_t shape_index) const;

  inline int GetRotationSpeed(const ValueSnapshot &snapshot,
                              size_t shape_index) const;

  inline int GetRotation(const ValueSnapshot &snapshot,
                         size_t shape_index) const;

  inline int GetTilt(const ValueSnapshot &snapshot, size_t shape_index) const;

  double GetBeamDirection(const ValueSnapshot &snapshot,
                          size_t shape_index) const;
  double GetBeamTilt(const ValueSnapshot &snapshot, size_t shape_index) const;

  Coordinate3D &GetPosition() { return position_; }
  const Coordinate3D &GetPosition() const { return position_; }
  Coordinate2D GetXY() const { return position_.XY(); }
  void SetXY(const Coordinate2D &xy) {
    position_.X() = xy.X();
    position_.Y() = xy.Y();
  }

  FixtureSymbol Symbol() const { return symbol_; }
  void SetSymbol(FixtureSymbol symbol) { symbol_ = symbol; }
  bool IsVisible() const { return symbol_ != FixtureSymbol::Hidden; }

  double Direction() const { return direction_; }
  void SetDirection(double direction) { direction_ = direction; }

  double StaticTilt() const { return static_tilt_; }
  void SetStaticTilt(double static_tilt) { static_tilt_ = static_tilt; }

  bool IsUpsideDown() const { return is_upside_down_; }
  void SetUpsideDown(bool is_upside_down) { is_upside_down_ = is_upside_down; }

  size_t ElectricPhase() const { return electric_phase_; }
  void SetElectricPhase(size_t phase) { electric_phase_ = phase; }

 private:
  Theatre &theatre_;
  const FixtureType &type_;
  Coordinate3D position_;
  double direction_ = 0.5 * M_PI;
  double static_tilt_ = kDefaultTilt;
  bool is_upside_down_ = false;
  FixtureSymbol symbol_;
  size_t electric_phase_ = 0;
  std::vector<std::unique_ptr<FixtureFunction>> functions_;
};

}  // namespace glight::theatre

#include "fixturetype.h"

namespace glight::theatre {

Color Fixture::GetColor(const ValueSnapshot &snapshot,
                        size_t shape_index) const {
  return type_.GetColor(*this, snapshot, shape_index);
}

int Fixture::GetRotationSpeed(const ValueSnapshot &snapshot,
                              size_t shape_index) const {
  return type_.GetRotationSpeed(*this, snapshot, shape_index);
}

int Fixture::GetRotation(const ValueSnapshot &snapshot,
                         size_t shape_index) const {
  return type_.GetPan(*this, snapshot, shape_index);
}

int Fixture::GetTilt(const ValueSnapshot &snapshot, size_t shape_index) const {
  return type_.GetTilt(*this, snapshot, shape_index);
}

}  // namespace glight::theatre

#endif
