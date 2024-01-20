#ifndef THEATRE_PROPERTY_SET_H_
#define THEATRE_PROPERTY_SET_H_

#include "property.h"

#include "../../theatre/controlvalue.h"
#include "../../theatre/folderobject.h"
#include "../../theatre/transition.h"

#include <memory>
#include <string>
#include <vector>

namespace glight::theatre {

class PropertySet {
 public:
  using iterator = std::vector<Property>::iterator;
  using const_iterator = std::vector<Property>::const_iterator;

  virtual ~PropertySet() = default;

  iterator begin() { return _properties.begin(); }
  const_iterator begin() const { return _properties.begin(); }

  iterator end() { return _properties.end(); }
  const_iterator end() const { return _properties.end(); }

  size_t size() const { return _properties.size(); }

  Property &operator[](size_t index) { return _properties[index]; }
  const Property &operator[](size_t index) const { return _properties[index]; }

  static std::unique_ptr<PropertySet> Make(FolderObject &object);

  static std::unique_ptr<PropertySet> Make(const FolderObject &object) {
    return Make(const_cast<FolderObject &>(object));
  }

  void SetControlValue(const Property &property, unsigned value) const {
    if (value > ControlValue::MaxUInt()) value = ControlValue::MaxUInt();
    setControlValue(*_object, property.set_index_, value);
  }

  unsigned GetControlValue(const Property &property) const {
    return getControlValue(*_object, property.set_index_);
  }

  void SetDuration(const Property &property, double value) const {
    setDuration(*_object, property.set_index_, value);
  }

  double GetDuration(const Property &property) const {
    return getDuration(*_object, property.set_index_);
  }

  void SetBool(const Property &property, double value) {
    setBool(*_object, property.set_index_, value);
  }

  bool GetBool(const Property &property) const {
    return getBool(*_object, property.set_index_);
  }

  void SetChoice(const Property &property, const std::string &value) {
    setChoice(*_object, property.set_index_, value);
  }

  std::string GetChoice(const Property &property) const {
    return getChoice(*_object, property.set_index_);
  }

  void SetInteger(const Property &property, int value) {
    setInteger(*_object, property.set_index_, value);
  }

  int GetInteger(const Property &property) const {
    return getInteger(*_object, property.set_index_);
  }

  void SetTransition(const Property &property, const Transition &value) {
    setTransition(*_object, property.set_index_, value);
  }

  Transition GetTransition(const Property &property) const {
    return getTransition(*_object, property.set_index_);
  }

  FolderObject &Object() const { return *_object; }

  Property &GetProperty(const std::string &name) {
    for (Property &p : _properties) {
      if (p.Name() == name) return p;
    }
    throw std::runtime_error("Property not found: " + name);
  }

  void AssignProperty(const Property &to, const Property &from,
                      const PropertySet &fromSet);

 protected:
  /**
   * Set the property
   * Sub-classes should override this method if they have control value
   * properties.
   * @param object The object for which to change the property value.
   * @param index The absolute index of this property. Note that other
   * property types are also counted; a property set with a bool and
   * a control value property will have index 0 for the bool and index
   * 1 for the control value.
   * @param value The new value of the property (between 0 and
   * ControlValue::MaxUInt()).
   */
  virtual void setControlValue(FolderObject &object, size_t index,
                               unsigned value) const {
    setterNotImplemented();
  }
  virtual unsigned getControlValue(const FolderObject &object,
                                   size_t index) const {
    getterNotImplemented();
    return 0;
  }

  virtual void setChoice(FolderObject &object, size_t index,
                         const std::string &value) const {
    setterNotImplemented();
  }
  virtual std::string getChoice(const FolderObject &object,
                                size_t index) const {
    getterNotImplemented();
    throw std::runtime_error("");
  }

  virtual void setDuration(FolderObject &object, size_t index,
                           double value) const {
    setterNotImplemented();
  }
  virtual double getDuration(const FolderObject &object, size_t index) const {
    getterNotImplemented();
    return 0;
  }

  virtual void setBool(FolderObject &object, size_t index, bool value) const {
    setterNotImplemented();
  }
  virtual bool getBool(const FolderObject &object, size_t index) const {
    getterNotImplemented();
    return 0;
  }

  virtual void setInteger(FolderObject &object, size_t index, int value) const {
    setterNotImplemented();
  }
  virtual int getInteger(const FolderObject &object, size_t index) const {
    getterNotImplemented();
    return 0;
  }

  virtual void setTransition(FolderObject &object, size_t index,
                             const Transition &value) const {
    setterNotImplemented();
  }
  virtual Transition getTransition(const FolderObject &object,
                                   size_t index) const {
    getterNotImplemented();
    return Transition();
  }

  size_t addProperty(const Property &property) {
    _properties.emplace_back(property);
    _properties.back().set_index_ = _properties.size() - 1;
    return _properties.back().set_index_;
  }
  size_t addProperty(Property &&property) {
    property.set_index_ = _properties.size();
    _properties.emplace_back(std::move(property));
    return _properties.back().set_index_;
  }

 private:
  void setterNotImplemented() const {
    throw std::runtime_error(
        "A method of the property set was called for "
        "which the set method was not implemented");
  }
  void getterNotImplemented() const {
    throw std::runtime_error(
        "A method of the property set was called for "
        "which the get method was not implemented");
  }
  FolderObject *_object;
  std::vector<Property> _properties;
};

class EmptyPS final : public PropertySet {
 public:
  EmptyPS() = default;
};

}  // namespace glight::theatre

#endif
