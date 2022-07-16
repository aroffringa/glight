#ifndef THEATRE_FOLDEROBJECT_H_
#define THEATRE_FOLDEROBJECT_H_

#include "namedobject.h"

namespace glight::theatre {

class Folder;

/**
 * @author Andre Offringa
 */
class FolderObject : public NamedObject {
 public:
  friend class Folder;

  FolderObject() : _parent(nullptr) {}
  FolderObject(const std::string &name) : NamedObject(name), _parent(nullptr) {}
  virtual ~FolderObject() {}

  FolderObject(const FolderObject &) = default;
  FolderObject(FolderObject &&) = default;
  FolderObject &operator=(const FolderObject &) = default;
  FolderObject &operator=(FolderObject &&) = default;

  std::string FullPath() const;

  bool IsRoot() const { return _parent == nullptr; }
  const Folder &Parent() const { return *_parent; }
  Folder &Parent() { return *_parent; }

 private:
  void SetParent(Folder &parent) { _parent = &parent; }

  Folder *_parent;
};

}  // namespace glight::theatre

#endif
