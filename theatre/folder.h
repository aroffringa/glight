#ifndef THEATRE_FOLDER_H_
#define THEATRE_FOLDER_H_

#include <algorithm>
#include <string>
#include <vector>

#include "folderobject.h"

#include "system/trackableptr.h"

namespace glight::theatre {

class Folder : public FolderObject {
 public:
  Folder() : FolderObject() {}

  Folder(const std::string &name) : FolderObject(name) {}

  /*Folder *CopyHierarchy(
      std::vector<std::unique_ptr<Folder>> &newFolders) const {
    newFolders.emplace_back(std::make_unique<Folder>(_name));
    Folder *copy = newFolders.back().get();
    for (const system::ObservingPtr<FolderObject>& object : _objects) {
      const Folder *child = dynamic_cast<const Folder *>(object.Get());
      if (child) {
        copy->_objects.emplace_back(child->CopyHierarchy(newFolders));
        copy->_objects.back()->SetParent(*copy);
      }
    }
    return copy;
  }*/

  /**
   * This also sets the parent of the specified object to this.
   */
  void Add(system::ObservingPtr<FolderObject> object) {
    FolderObject *ptr = object.Get();
    if (GetChildIfExists(ptr->Name()))
      throw std::runtime_error(
          "Trying to add object " + ptr->Name() + " to folder " + Name() +
          ", but an object with that name already exists in this folder");
    _objects.emplace_back(std::move(object));
    ptr->SetParent(*this);
  }

  void Remove(FolderObject &object) {
    std::vector<system::ObservingPtr<FolderObject>>::iterator srciter =
        std::find(_objects.begin(), _objects.end(), &object);
    _objects.erase(srciter);
  }

  void MoveUp(FolderObject &object) {
    std::vector<system::ObservingPtr<FolderObject>>::iterator srciter =
        std::find(_objects.begin(), _objects.end(), &object);
    if (srciter != _objects.begin() && srciter != _objects.end()) {
      std::vector<system::ObservingPtr<FolderObject>>::iterator previous =
          srciter;
      --previous;
      std::swap(*previous, *srciter);
    }
  }

  void MoveDown(FolderObject &object) {
    std::vector<system::ObservingPtr<FolderObject>>::iterator srciter =
        std::find(_objects.begin(), _objects.end(), &object);
    if (srciter != _objects.end()) {
      std::vector<system::ObservingPtr<FolderObject>>::iterator next = srciter;
      ++next;
      if (_objects.end() != next) std::swap(*next, *srciter);
    }
  }

  const std::vector<system::ObservingPtr<FolderObject>> Children() const {
    return _objects;
  }

  Folder *FollowDown(const std::string &path) {
    if (path.empty())
      return this;
    else
      return followDown(path, 0);
  }

  Folder *FollowDown(std::string &&path) {
    if (path.empty())
      return this;
    else
      return followDown(std::move(path), 0);
  }

  FolderObject *FollowRelPath(const std::string &path);
  FolderObject *FollowRelPath(std::string &&path);

  FolderObject &GetChild(const std::string &name) {
    return *FindNamedObject(_objects, name);
  }

  FolderObject *GetChildIfExists(const std::string &name) {
    return FindNamedObjectIfExists(_objects, name).Get();
  }

  const FolderObject &GetChild(const std::string &name) const {
    return *FindNamedObject(_objects, name);
  }

  const FolderObject *GetChildIfExists(const std::string &name) const {
    return FindNamedObjectIfExists(_objects, name).Get();
  }

  std::string GetAvailableName(const std::string &prefix) const {
    bool nameAvailable;
    size_t nameNumber = 0;
    do {
      nameAvailable = true;
      ++nameNumber;
      for (const system::ObservingPtr<FolderObject> &object : _objects) {
        if (object->Name() == prefix + std::to_string(nameNumber)) {
          nameAvailable = false;
          break;
        }
      }
    } while (!nameAvailable);
    return prefix + std::to_string(nameNumber);
  }

  static void Move(system::ObservingPtr<FolderObject> object,
                   Folder &destination) {
    FolderObject *ptr = object.Get();
    if (&destination != &ptr->Parent()) {
      if (destination.GetChildIfExists(ptr->Name()))
        throw std::runtime_error(
            "Can't move object: the destination folder already contains an "
            "object with the same name");
      ptr->_parent->Remove(*ptr);

      destination._objects.emplace_back(std::move(object));
      ptr->_parent = &destination;
    }
  }

 private:
  Folder *followDown(const std::string &path, size_t strPos) const {
    auto sep = std::find(path.begin() + strPos, path.end(), '/');
    std::string subpath;
    if (sep == path.end()) {
      FolderObject *obj =
          FindNamedObjectIfExists(_objects, path.substr(strPos)).Get();
      return dynamic_cast<Folder *>(obj);
    } else {
      FolderObject *obj =
          FindNamedObjectIfExists(
              _objects, path.substr(strPos, sep - path.begin() - strPos))
              .Get();
      Folder *folder = dynamic_cast<Folder *>(obj);
      if (folder)
        return folder->followDown(path, sep + 1 - path.begin());
      else
        return nullptr;
    }
  }

  Folder *followDown(std::string &&path, size_t strPos) const {
    auto sep = std::find(path.begin() + strPos, path.end(), '/');
    std::string subpath;
    if (sep == path.end()) {
      FolderObject *obj =
          FindNamedObjectIfExists(_objects, std::move(path).substr(strPos))
              .Get();
      return dynamic_cast<Folder *>(obj);
    } else {
      FolderObject *obj =
          FindNamedObjectIfExists(
              _objects, path.substr(strPos, sep - path.begin() - strPos))
              .Get();
      Folder *folder = dynamic_cast<Folder *>(obj);
      if (folder)
        return folder->followDown(std::move(path), sep + 1 - path.begin());
      else
        return nullptr;
    }
  }

  std::vector<system::ObservingPtr<FolderObject>> _objects;
};

}  // namespace glight::theatre

#endif
