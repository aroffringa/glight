#include "theatre.h"

#include "fixture.h"
#include "folder.h"

#include <cassert>
#include <cmath>
#include <sstream>

namespace glight::theatre {

using system::TrackablePtr;

void Theatre::Clear() {
  _fixtures.clear();
  _fixtureTypes.clear();
}

const TrackablePtr<Fixture> &Theatre::AddFixture(const FixtureType &type) {
  // Find free name
  std::string ext = "A";
  std::string prefix = type.ShortName();
  if (!prefix.empty()) prefix = prefix + " ";
  while (true) {
    if (!NamedObject::Contains(_fixtures, prefix + ext)) {
      break;
    }
    bool ready = false;
    do {
      for (size_t i = 0; i != ext.size(); ++i) {
        char &c = ext[ext.size() - i - 1];
        if (c != 'Z') {
          ++c;
          ready = true;
          break;
        } else {
          c = 'A';
        }
      }
      if (!ready) {
        // No name available with current string length, increase length
        ext.assign(ext.size() + 1, 'A');
        ready = true;
      }
    } while (!ready);
  }
  TrackablePtr<Fixture> &f = _fixtures.emplace_back(
      system::MakeTrackable<Fixture>(*this, type, prefix + ext));
  NotifyDmxChange();
  return f;
}

const TrackablePtr<FixtureType> &Theatre::AddFixtureType(
    StockFixture fixtureClass) {
  return _fixtureTypes.emplace_back(
      system::MakeTrackable<FixtureType>(fixtureClass));
}

const TrackablePtr<FixtureType> &Theatre::AddFixtureType(
    const FixtureType &fixture_type) {
  return _fixtureTypes.emplace_back(
      system::MakeTrackable<FixtureType>(fixture_type));
}

bool Theatre::Contains(Fixture &fixture) const {
  for (const system::TrackablePtr<Fixture> &f : _fixtures) {
    if (f.Get() == &fixture) return true;
  }
  return false;
}

Fixture &Theatre::GetFixture(const std::string &name) const {
  return *NamedObject::FindNamedObject(_fixtures, name);
}

system::ObservingPtr<Fixture> Theatre::GetFixturePtr(
    const std::string &name) const {
  return NamedObject::FindNamedObject(_fixtures, name).GetObserver();
}

const system::TrackablePtr<FixtureType> &Theatre::GetFixtureType(
    const std::string &name) const {
  return NamedObject::FindNamedObject(_fixtureTypes, name);
}

FixtureFunction &Theatre::GetFixtureFunction(const std::string &name) const {
  for (const system::TrackablePtr<Fixture> &f : _fixtures) {
    const std::vector<std::unique_ptr<FixtureFunction>> &functions =
        f->Functions();
    for (const std::unique_ptr<FixtureFunction> &function : functions) {
      if (function->Name() == name) return *function;
    }
  }
  throw std::runtime_error(
      std::string("Can not find fixture function with name ") + name);
}

void Theatre::RemoveFixture(const Fixture &fixture) {
  const size_t fIndex = NamedObject::FindIndex(_fixtures, &fixture);
  _fixtures.erase(_fixtures.begin() + fIndex);
}

void Theatre::RemoveFixtureType(const FixtureType &fixtureType) {
  size_t i = 0;
  while (i != _fixtures.size()) {
    // Go backward through the list, as they might be removed
    const size_t fIndex = _fixtures.size() - 1 - i;
    Fixture &f = *_fixtures[fIndex];
    if (&f.Type() == &fixtureType) {
      _fixtures.erase(_fixtures.begin() + fIndex);
    } else {
      ++i;
    }
  }
  const size_t ftIndex = NamedObject::FindIndex(_fixtureTypes, &fixtureType);
  _fixtureTypes[ftIndex]->Parent().Remove(*_fixtureTypes[ftIndex]);
  _fixtureTypes.erase(_fixtureTypes.begin() + ftIndex);
}

void Theatre::SwapFixturePositions(const Fixture &fixture_a,
                                   const Fixture &fixture_b) {
  system::TrackablePtr<Fixture> *a = nullptr;
  system::TrackablePtr<Fixture> *b = nullptr;
  for (system::TrackablePtr<Fixture> &f : _fixtures) {
    if (f.Get() == &fixture_a) a = &f;
    if (f.Get() == &fixture_b) b = &f;
  }
  assert(a);
  assert(b);
  swap(*a, *b);
}

bool Theatre::IsUsed(const FixtureType &fixtureType) const {
  for (const system::TrackablePtr<Fixture> &f : _fixtures) {
    if (&f->Type() == &fixtureType) {
      return true;
    }
  }
  return false;
}

void Theatre::NotifyDmxChange() {
  unsigned highest = 0;
  for (const system::TrackablePtr<Fixture> &fixture : _fixtures) {
    std::vector<unsigned> channels = fixture->GetChannels();
    for (unsigned channel : channels) {
      if (channel > highest) highest = channel;
    }
  }
  _highestChannel = highest;
}

Position Theatre::GetFreePosition() const {
  const size_t rowLength = 10.0;
  size_t n = _fixtures.size() * 2;
  std::unique_ptr<bool[]> available(new bool[n]);
  std::fill_n(available.get(), n, true);

  for (const system::TrackablePtr<Fixture> &fixture : _fixtures) {
    double x = fixture->GetPosition().X();
    if (x < rowLength) {
      double index = x + fixture->GetPosition().Y() * rowLength;
      size_t midIndex = round(index);
      if (midIndex < n) available[midIndex] = false;
      if (midIndex - index > 0.01) {
        size_t secIndex = index + 1;
        if (secIndex < n) available[secIndex] = false;
      } else if (index - midIndex > 0.01) {
        size_t secIndex = index - 1;
        if (secIndex < n) available[secIndex] = false;
      }
    }
  }
  for (size_t i = 0; i != n; ++i) {
    if (available[i]) return Position(i % rowLength, i / rowLength);
  }
  return Position(n % rowLength, n / rowLength);
}

Position Theatre::Extend() const {
  Position extend;
  for (const system::TrackablePtr<Fixture> &fixture : _fixtures) {
    const double right = fixture->GetPosition().X() + 1.0;
    const double bottom = fixture->GetPosition().Y() + 1.0;
    if (right > extend.X()) extend.X() = right;
    if (bottom > extend.Y()) extend.Y() = bottom;
  }
  return extend;
}

}  // namespace glight::theatre
