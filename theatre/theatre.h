#ifndef THEATRE_THEATRE_H_
#define THEATRE_THEATRE_H_

#include <memory>
#include <vector>

#include "fixture.h"
#include "fixturetype.h"
#include "position.h"

namespace glight::theatre {

class Fixture;
class FixtureType;

/**
 * @author Andre Offringa
 */
class Theatre {
 public:
  Theatre() = default;
  Theatre(const Theatre &source);

  void Clear();

  Fixture &AddFixture(const FixtureType &type);
  FixtureType &AddFixtureType(StockFixture fixtureClass);
  FixtureType &AddFixtureType(const FixtureType &type);

  bool Contains(Fixture &fixture) const;

  const std::vector<std::unique_ptr<Fixture>> &Fixtures() const {
    return _fixtures;
  }
  const std::vector<std::unique_ptr<FixtureType>> &FixtureTypes() const {
    return _fixtureTypes;
  }

  Fixture &GetFixture(const std::string &name) const;
  FixtureType &GetFixtureType(const std::string &name) const;
  FixtureFunction &GetFixtureFunction(const std::string &name) const;

  void RemoveFixture(const Fixture &fixture);
  void RemoveFixtureType(const FixtureType &fixtureType);

  bool IsUsed(const FixtureType &fixtureType) const;

  unsigned HighestChannel() const { return _highestChannel; }
  unsigned FirstFreeChannel() const {
    return _fixtures.empty() ? 0 : _highestChannel + 1;
  }
  void NotifyDmxChange();

  Position GetFreePosition() const;

  Position Extend() const;

 private:
  std::vector<std::unique_ptr<Fixture>> _fixtures;
  std::vector<std::unique_ptr<FixtureType>> _fixtureTypes;
  unsigned _highestChannel = 0;
};

}  // namespace glight::theatre

#endif
