#ifndef THEATRE_FIXTURE_H_
#define THEATRE_FIXTURE_H_

#include <set>

#include "color.h"
#include "fixturefunction.h"
#include "fixturesymbol.h"
#include "namedobject.h"
#include "position.h"

namespace glight::theatre {

class FixtureType;
class Position;
class Theatre;
class ValueSnapshot;

/**
 * @author Andre Offringa
 */
class Fixture : public NamedObject {
 public:
  Fixture(Theatre &theatre, const FixtureType &type, const std::string &name);
  Fixture(const Fixture &source, Theatre &theatre);

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

  void IncChannel();

  void DecChannel();

  void SetChannel(DmxChannel dmx_channel);

  void ClearFunctions() { functions_.clear(); }
  FixtureFunction &AddFunction(FunctionType type) {
    functions_.emplace_back(new FixtureFunction(theatre_, type));
    return *functions_.back();
  }
  inline Color GetColor(const ValueSnapshot &snapshot,
                        size_t shape_index) const;

  inline int GetRotationSpeed(const ValueSnapshot &snapshot,
                              size_t shape_index) const;

  inline int GetRotation(const ValueSnapshot &snapshot,
                         size_t shape_index) const;

  inline int GetTilt(const ValueSnapshot &snapshot, size_t shape_index) const;

  Position &GetPosition() { return position_; }
  const Position &GetPosition() const { return position_; }

  FixtureSymbol Symbol() const { return symbol_; }
  void SetSymbol(FixtureSymbol symbol) { symbol_ = symbol; }
  bool IsVisible() const { return symbol_ != FixtureSymbol::Hidden; }

  double Direction() const { return direction_; }
  void SetDirection(double direction) { direction_ = direction; }

  double Tilt() const { return tilt_; }
  void SetTilt(double tilt) { tilt_ = tilt; }

  bool IsUpsideDown() const { return is_upside_down_; }
  void SetUpsideDown(bool is_upside_down) { is_upside_down_ = is_upside_down; }

 private:
  Theatre &theatre_;
  const FixtureType &type_;
  Position position_;
  double direction_ = 0.5 * M_PI;
  double tilt_ = 0.0;
  bool is_upside_down_ = false;
  FixtureSymbol symbol_;
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
