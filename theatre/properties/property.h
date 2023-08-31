#ifndef THEATRE_PROPERTY_H_
#define THEATRE_PROPERTY_H_

#include "../../theatre/folderobject.h"

#include "../../theatre/effects/thresholdeffect.h"

#include <string>

namespace glight::theatre {

enum class PropertyType {
  Choice,
  ControlValue,
  Duration,
  Boolean,
  Integer,
  Transition
};

class Property final {
 public:
  Property(const std::string &name, const std::string &description,
           PropertyType type)
      : _type(type), _setIndex(0), _name(name), _description(description) {}

  Property(const std::string &name, const std::string &description,
           const std::vector<std::pair<std::string, std::string>> &options)
      : _type(PropertyType::Choice),
        _setIndex(0),
        _name(name),
        _description(description),
        _options(options) {}

  PropertyType GetType() const { return _type; }

  const std::string &Name() const { return _name; }

  const std::string &Description() const { return _description; }

  size_t OptionCount() const { return _options.size(); }
  const std::string &OptionName(size_t index) const {
    return _options[index].first;
  }
  const std::string &OptionDescription(size_t index) const {
    return _options[index].second;
  }

 private:
  friend class PropertySet;

  PropertyType _type;
  size_t _setIndex;
  std::string _name, _description;
  std::vector<std::pair<std::string, std::string>> _options;
};

}  // namespace glight::theatre

#endif
