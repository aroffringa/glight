#ifndef THEATRE_THEATRE_H_
#define THEATRE_THEATRE_H_

#include <memory>
#include <vector>

#include "fixture.h"
#include "fixturetype.h"
#include "position.h"

#include "system/observableptr.h"

namespace glight::theatre {

/**
 * @author Andre Offringa
 */
class Theatre {
 public:
  Theatre() = default;
  // Theatre(const Theatre &source);

  void Clear();

  Fixture &AddFixture(const FixtureType &type);
  FixtureType &AddFixtureType(StockFixture fixtureClass);
  FixtureType &AddFixtureType(const FixtureType &type);

  bool Contains(Fixture &fixture) const;

  const std::vector<system::ObservablePtr<Fixture>> &Fixtures() const {
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

  void SwapFixturePositions(const Fixture &fixture_a, const Fixture &fixture_b);
  bool IsUsed(const FixtureType &fixtureType) const;

  /**
   * Highest channel used (over all universes).
   */
  unsigned HighestChannel() const { return _highestChannel; }
  DmxChannel FirstFreeChannel() const {
    // TODO support multiple universes
    return DmxChannel(_fixtures.empty() ? 0 : _highestChannel + 1, 0);
  }
  void NotifyDmxChange();

  Position GetFreePosition() const;
  Position Extend() const;

  double Width() const { return width_; }
  void SetWidth(double width) { width_ = width; }
  double Depth() const { return depth_; }
  void SetDepth(double depth) { depth_ = depth; }
  double Height() const { return height_; }
  void SetHeight(double height) { height_ = height; }
  double FixtureSymbolSize() const { return fixture_symbol_size_; }
  void SetFixtureSymbolSize(double fixture_symbol_size) {
    fixture_symbol_size_ = fixture_symbol_size;
  }

 private:
  double width_ = 10.0;
  double depth_ = 10.0;
  double height_ = 10.0;
  double fixture_symbol_size_ = 0.5;
  std::vector<system::ObservablePtr<Fixture>> _fixtures;
  std::vector<std::unique_ptr<FixtureType>> _fixtureTypes;
  unsigned _highestChannel = 0;
};

}  // namespace glight::theatre

#endif
