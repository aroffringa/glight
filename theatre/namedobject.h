#ifndef THEATRE_NAMEDOBJECT_H_
#define THEATRE_NAMEDOBJECT_H_

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <sigc++/signal.h>

#include "system/trackableptr.h"

namespace glight::theatre {

/**
        @author Andre Offringa
*/
class NamedObject {
 public:
  friend class Folder;

  NamedObject() : _name() {}
  NamedObject(const std::string &name) : _name(name) {}
  NamedObject(std::string &&name) : _name(std::move(name)) {}
  NamedObject(std::string_view name) : _name(name) {}
  NamedObject(const char name[]) : _name(name) {}
  virtual ~NamedObject() { _signalDelete(); }

  NamedObject(const NamedObject &source)
      : _name(source._name), _signalDelete() {}
  NamedObject(NamedObject &&source) : _name(source._name), _signalDelete() {}
  NamedObject &operator=(const NamedObject &source) {
    _signalDelete();
    _signalDelete.clear();
    _name = source._name;
    return *this;
  }
  NamedObject &operator=(NamedObject &&source) {
    _signalDelete();
    _signalDelete.clear();
    _name = source._name;
    return *this;
  }

  const std::string &Name() const { return _name; }
  void SetName(const std::string &name) { _name = name; }

  template <typename NamedObjectType>
  static NamedObjectType *FindNamedObjectIfExists(
      const std::vector<std::unique_ptr<NamedObjectType>> &container,
      std::string_view name) {
    for (const std::unique_ptr<NamedObjectType> &obj : container) {
      if (obj->_name == name) return obj.get();
    }
    return nullptr;
  }

  template <typename NamedObjectType>
  static const system::TrackablePtr<NamedObjectType> *FindNamedObjectIfExists(
      const std::vector<system::TrackablePtr<NamedObjectType>> &container,
      std::string_view name) {
    for (const system::TrackablePtr<NamedObjectType> &obj : container) {
      if (obj->_name == name) return &obj;
    }
    return nullptr;
  }

  template <typename NamedObjectType>
  static system::ObservingPtr<NamedObjectType> FindNamedObjectIfExists(
      const std::vector<system::ObservingPtr<NamedObjectType>> &container,
      std::string_view name) {
    for (const system::ObservingPtr<NamedObjectType> &obj : container) {
      if (obj->_name == name) return obj;
    }
    return {};
  }

  template <typename NamedObjectType>
  static NamedObjectType *FindNamedObjectIfExists(
      const std::vector<NamedObjectType *> &container, std::string_view name) {
    for (NamedObjectType *obj : container) {
      if (obj->_name == name) return obj;
    }
    return nullptr;
  }

  template <typename NamedObjectType>
  static NamedObjectType &FindNamedObject(
      const std::vector<std::unique_ptr<NamedObjectType>> &container,
      std::string_view name) {
    NamedObjectType *obj = FindNamedObjectIfExists(container, name);
    if (obj)
      return *obj;
    else
      throw std::runtime_error("Could not find named object " +
                               std::string(name) + " in container.");
  }

  template <typename NamedObjectType>
  static const system::TrackablePtr<NamedObjectType> &FindNamedObject(
      const std::vector<system::TrackablePtr<NamedObjectType>> &container,
      std::string_view name) {
    const system::TrackablePtr<NamedObjectType> *obj =
        FindNamedObjectIfExists(container, name);
    if (obj)
      return *obj;
    else
      throw std::runtime_error("Could not find named object " +
                               std::string(name) + " in container.");
  }

  template <typename NamedObjectType>
  static const system::ObservingPtr<NamedObjectType> FindNamedObject(
      const std::vector<system::ObservingPtr<NamedObjectType>> &container,
      std::string_view name) {
    system::ObservingPtr<NamedObjectType> obj =
        FindNamedObjectIfExists(container, name);
    if (obj)
      return obj;
    else
      throw std::runtime_error("Could not find named object " +
                               std::string(name) + " in container.");
  }

  template <typename NamedObjectType>
  static NamedObjectType &FindNamedObject(
      const std::vector<NamedObjectType *> &container, std::string_view name) {
    NamedObjectType *obj = FindNamedObjectIfExists(container, name);
    if (obj)
      return *obj;
    else
      throw std::runtime_error("Could not find named object " +
                               std::string(name) + " in container.");
  }

  template <typename ObjectType>
  static size_t FindIndex(
      const std::vector<std::unique_ptr<ObjectType>> &container,
      const ObjectType *element) {
    for (size_t i = 0; i != container.size(); ++i) {
      if (container[i].get() == element) {
        return i;
      }
    }
    throw std::runtime_error("Could not find object in container.");
  }

  template <typename ObjectType>
  static size_t FindIndex(
      const std::vector<system::TrackablePtr<ObjectType>> &container,
      const ObjectType *element) {
    for (size_t i = 0; i != container.size(); ++i) {
      if (container[i].Get() == element) {
        return i;
      }
    }
    throw std::runtime_error("Could not find object in container.");
  }

  template <typename ObjectType>
  static bool Contains(
      const std::vector<std::unique_ptr<ObjectType>> &container,
      const ObjectType *element) {
    for (const std::unique_ptr<ObjectType> &obj : container) {
      if (obj.get() == &element) return true;
    }
    return false;
  }

  template <typename ObjectType>
  static bool Contains(
      const std::vector<system::TrackablePtr<ObjectType>> &container,
      const ObjectType *element) {
    for (const std::unique_ptr<ObjectType> &obj : container) {
      if (obj.Get() == &element) return true;
    }
    return false;
  }

  template <typename ObjectType>
  static bool Contains(
      const std::vector<std::unique_ptr<ObjectType>> &container,
      std::string_view name) {
    for (const std::unique_ptr<ObjectType> &obj : container) {
      if (obj->_name == name) return true;
    }
    return false;
  }

  template <typename ObjectType>
  static bool Contains(
      const std::vector<system::TrackablePtr<ObjectType>> &container,
      std::string_view name) {
    for (const system::TrackablePtr<ObjectType> &obj : container) {
      if (obj->_name == name) return true;
    }
    return false;
  }

  sigc::signal<void()> &SignalDelete() { return _signalDelete; }

 private:
  [[no_unique_address]] std::string _name;
  sigc::signal<void()> _signalDelete;
};

}  // namespace glight::theatre

#endif
