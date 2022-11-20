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
    return _functions;
  }

  const FixtureType &Type() const { return _type; }

  std::vector<unsigned> GetChannels() const {
    std::vector<unsigned> channels;
    for (const std::unique_ptr<FixtureFunction> &ff : _functions) {
      channels.emplace_back(ff->FirstChannel().Channel());
      if (!ff->IsSingleChannel())
        channels.emplace_back(ff->FirstChannel().Channel() + 1);
    }
    return channels;
  }

  void IncChannel();

  void DecChannel();

  void SetChannel(unsigned dmxChannel);

  void ClearFunctions() { _functions.clear(); }
  FixtureFunction &AddFunction(FunctionType type) {
    _functions.emplace_back(new FixtureFunction(_theatre, type));
    return *_functions.back();
  }
  inline Color GetColor(const ValueSnapshot &snapshot, size_t shapeIndex) const;

  inline int GetRotationSpeed(const ValueSnapshot &snapshot,
                              size_t shapeIndex) const;

  Position &GetPosition() { return _position; }
  const Position &GetPosition() const { return _position; }

  FixtureSymbol Symbol() const { return _symbol; }
  void SetSymbol(FixtureSymbol symbol) { _symbol = symbol; }
  bool IsVisible() const { return _symbol != FixtureSymbol::Hidden; }

  double Direction() const { return _direction; }
  void SetDirection(double direction) { _direction = direction; }

  double Tilt() const { return _tilt; }
  void SetTilt(double tilt) { _tilt = tilt; }

 private:
  Theatre &_theatre;
  const FixtureType &_type;
  Position _position;
  double _direction = 0.5 * M_PI;
  double _tilt = 0.0;
  FixtureSymbol _symbol;
  std::vector<std::unique_ptr<FixtureFunction>> _functions;
};

}  // namespace glight::theatre

#include "fixturetype.h"

namespace glight::theatre {

Color Fixture::GetColor(const ValueSnapshot &snapshot,
                        size_t shapeIndex) const {
  return _type.GetColor(*this, snapshot, shapeIndex);
}

int Fixture::GetRotationSpeed(const ValueSnapshot &snapshot,
                              size_t shapeIndex) const {
  return _type.GetRotationSpeed(*this, snapshot, shapeIndex);
}

}  // namespace glight::theatre

#endif
