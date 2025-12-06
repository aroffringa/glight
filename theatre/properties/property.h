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
  TimePattern,
  Transition
};

class Property final {
 public:
  Property(const std::string &name, const std::string &description,
           PropertyType type)
      : type_(type), set_index_(0), name_(name), description_(description) {}

  /**
   * Create a choice property, similar to an enum.
   * @param options holds the names and descriptions of the choices.
   */
  Property(const std::string &name, const std::string &description,
           const std::vector<std::pair<std::string, std::string>> &options)
      : type_(PropertyType::Choice),
        set_index_(0),
        name_(name),
        description_(description),
        options_(options) {}

  PropertyType GetType() const { return type_; }

  const std::string &Name() const { return name_; }

  const std::string &Description() const { return description_; }

  size_t OptionCount() const { return options_.size(); }
  const std::string &OptionName(size_t index) const {
    return options_[index].first;
  }
  const std::string &OptionDescription(size_t index) const {
    return options_[index].second;
  }

 private:
  friend class PropertySet;

  PropertyType type_;
  size_t set_index_;
  std::string name_;
  std::string description_;
  // Vector with names and descriptions
  std::vector<std::pair<std::string, std::string>> options_;
};

}  // namespace glight::theatre

#endif
