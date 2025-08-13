#ifndef THEATRE_THEATRE_H_
#define THEATRE_THEATRE_H_

#include <memory>
#include <vector>

#include "coordinate3d.h"
#include "dmxchannel.h"
#include "forwards.h"
#include "stockfixture.h"

#include "system/trackableptr.h"

namespace glight::theatre {

/**
 * @author Andre Offringa
 */
class Theatre {
 public:
  Theatre();
  ~Theatre();

  void Clear();

  const system::TrackablePtr<Fixture> &AddFixture(const FixtureMode &mode);
  system::ObservingPtr<Fixture> AddFixturePtr(const FixtureMode &mode) {
    return AddFixture(mode).GetObserver();
  }

  const system::TrackablePtr<FixtureType> &AddFixtureType(StockFixture stock_fixture);
  system::ObservingPtr<FixtureType> AddFixtureTypePtr(StockFixture stock_fixture) {
    return AddFixtureType(stock_fixture).GetObserver();
  }

  const system::TrackablePtr<FixtureType> &AddFixtureType(
      system::TrackablePtr<FixtureType>&& type);
  system::ObservingPtr<FixtureType> AddFixtureTypePtr(system::TrackablePtr<FixtureType>&& type) {
    return AddFixtureType(std::move(type)).GetObserver();
  }

  bool Contains(Fixture &fixture) const;

  const std::vector<system::TrackablePtr<Fixture>> &Fixtures() const {
    return _fixtures;
  }
  const std::vector<system::TrackablePtr<FixtureType>> &FixtureTypes() const {
    return _fixtureTypes;
  }

  Fixture &GetFixture(const std::string &name) const;
  system::ObservingPtr<Fixture> GetFixturePtr(const std::string &name) const;
  const system::TrackablePtr<FixtureType> &GetFixtureType(
      const std::string &name) const;
  system::ObservingPtr<FixtureType> GetFixtureTypePtr(
      const std::string &name) const {
    return GetFixtureType(name).GetObserver();
  }
  FixtureFunction &GetFixtureFunction(const std::string &name) const;

  void RemoveFixture(const Fixture &fixture);
  void RemoveFixtureType(const FixtureType &fixtureType);

  void SwapFixturePositions(const Fixture &fixture_a, const Fixture &fixture_b);
  bool IsUsed(const FixtureType &fixture_type) const;
  bool IsUsed(const FixtureMode &fixture_mode) const;

  /**
   * Highest channel used (over all universes).
   */
  unsigned HighestChannel() const { return _highestChannel; }
  DmxChannel FirstFreeChannel() const {
    // TODO support multiple universes
    return DmxChannel(_fixtures.empty() ? 0 : _highestChannel + 1, 0);
  }
  void NotifyDmxChange();

  Coordinate3D GetFreePosition() const;
  Coordinate2D Extend() const;

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
  std::vector<system::TrackablePtr<Fixture>> _fixtures;
  std::vector<system::TrackablePtr<FixtureType>> _fixtureTypes;
  unsigned _highestChannel = 0;
};

}  // namespace glight::theatre

#endif
