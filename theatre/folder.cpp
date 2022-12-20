#include "folder.h"
#include "folderoperations.h"

namespace glight::theatre {

FolderObject *Folder::FollowRelPath(const std::string &path) {
  Folder *parentFolder = FollowDown(folders::ParentPath(path));
  if (parentFolder)
    return parentFolder->GetChildIfExists(folders::LastName(path));
  else
    return nullptr;
}

FolderObject *Folder::FollowRelPath(std::string &&path) {
  Folder *parentFolder = FollowDown(folders::ParentPath(path));
  if (parentFolder)
    return parentFolder->GetChildIfExists(folders::LastName(std::move(path)));
  else
    return nullptr;
}

}  // namespace glight::theatre
