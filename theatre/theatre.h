#ifndef THEATRE_H
#define THEATRE_H

#include <memory>
#include <vector>

#include "fixture.h"
#include "fixturetype.h"
#include "position.h"

/**
        @author Andre Offringa
*/
class Theatre {
 public:
  Theatre() : _highestChannel(0) {}
  ~Theatre() { Clear(); }
  Theatre(const Theatre &source);

  void Clear();

  class Fixture &AddFixture(const FixtureType &type);
  class FixtureType &AddFixtureType(FixtureClass fixtureClass);

  bool Contains(Fixture &fixture) const;

  const std::vector<std::unique_ptr<class Fixture>> &Fixtures() const {
    return _fixtures;
  }
  const std::vector<std::unique_ptr<class FixtureType>> &FixtureTypes() const {
    return _fixtureTypes;
  }

  class Fixture &GetFixture(const std::string &name) const;
  class FixtureType &GetFixtureType(const std::string &name) const;
  class FixtureFunction &GetFixtureFunction(const std::string &name) const;

  void RemoveFixture(Fixture &fixture);
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
  std::vector<std::unique_ptr<class Fixture>> _fixtures;
  std::vector<std::unique_ptr<class FixtureType>> _fixtureTypes;
  unsigned _highestChannel;
};

#endif
