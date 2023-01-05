#ifndef THEATRE_FIXTURE_GROUP_H_
#define THEATRE_FIXTURE_GROUP_H_

#include <map>

#include "fixture.h"
#include "folderobject.h"

#include <sigc++/connection.h>

namespace glight::theatre {

class FixtureGroup : public FolderObject {
 public:
  FixtureGroup() = default;
  FixtureGroup(const std::string& name) : FolderObject(name) {}
  ~FixtureGroup() { Clear(); }

  std::vector<Fixture*> Fixtures() const {
    std::vector<Fixture*> list;
    list.reserve(fixtures_.size());
    for (const Item& item : fixtures_) {
      list.emplace_back(item.first);
    }
    return list;
  }

  void Clear() {
    for (Item& item : fixtures_) {
      item.second.disconnect();
    }
    fixtures_.clear();
  }

  bool Insert(Fixture& fixture) {
    const auto [new_item, is_added] = fixtures_.emplace(
        std::piecewise_construct, std::forward_as_tuple(&fixture),
        std::forward_as_tuple());
    if (is_added) {
      new_item->second =
          fixture.SignalDelete().connect([&]() { Remove(fixture); });
    }
    return is_added;
  }

  bool Remove(const Fixture& fixture) {
    const iterator item = fixtures_.find(const_cast<Fixture*>(&fixture));
    if (item != fixtures_.end()) {
      item->second.disconnect();
      fixtures_.erase(item);
      return true;
    } else {
      return false;
    }
  }

  bool Contains(const Fixture& fixture) {
    return fixtures_.count(const_cast<Fixture*>(&fixture)) != 0;
  }

  size_t Size() const { return fixtures_.size(); }

  bool Empty() const { return fixtures_.empty(); }

 private:
  using iterator = std::map<Fixture*, sigc::connection>::iterator;
  using Item = std::pair<Fixture* const, sigc::connection>;

  std::map<Fixture*, sigc::connection> fixtures_;
};

}  // namespace glight::theatre

#endif
