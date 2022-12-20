#ifndef THEATRE_FOLDER_OPERATIONS_H_
#define THEATRE_FOLDER_OPERATIONS_H_

#include <algorithm>
#include <string>
#include <vector>

#include "folderobject.h"

namespace glight::theatre::folders {

inline std::string ParentPath(const std::string &path) {
  auto separator = std::find(path.rbegin(), path.rend(), '/');
  if (separator == path.rend())
    return std::string();
  else {
    size_t nChar = path.size() - (separator - path.rbegin()) - 1;
    return path.substr(0, nChar);
  }
}

inline std::string ParentPath(std::string &&path) {
  auto separator = std::find(path.rbegin(), path.rend(), '/');
  if (separator == path.rend())
    return std::string();
  else {
    size_t nChar = path.size() - (separator - path.rbegin()) - 1;
    path.resize(nChar);
    return std::move(path);
  }
}

inline std::string LastName(const std::string &path) {
  auto separator = std::find(path.rbegin(), path.rend(), '/');
  if (separator == path.rend())
    return path;
  else {
    size_t sepIndex = path.size() - (separator - path.rbegin());
    return path.substr(sepIndex);
  }
}

inline std::string RemoveRoot(const std::string &path) {
  auto separator = std::find(path.begin(), path.end(), '/');
  if (separator == path.end())
    return std::string();
  else {
    return path.substr(separator - path.begin() + 1);
  }
}

inline size_t Depth(const std::string &path) {
  if (path.empty())
    return 0;
  else
    return std::count(path.begin(), path.end(), '/') + 1;
}

inline std::string Root(const std::string &path) {
  auto separator = std::find(path.begin(), path.end(), '/');
  if (separator == path.end())
    return path;
  else {
    return std::string(path.begin(), separator);
  }
}

inline std::string ShortDescription(const std::string &path,
                                    size_t max_length) {
  if (path.size() <= max_length)
    return path;
  else if (max_length <= 4)
    return std::string("[..]").substr(0, max_length);
  else {
    const size_t depth = Depth(path);
    if (depth <= 1) {
      return path.substr(0, max_length - 4) + "[..]";
    } else {
      const std::string root = Root(path);
      std::string scratch = RemoveRoot(path);
      if (scratch.size() + 5 <= max_length)
        return root.substr(0, max_length - 5 - scratch.size()) + "[..]/" +
               scratch;
      else if (depth == 2)
        return "[..]" +
               scratch.substr(scratch.size() + 4 - max_length, max_length - 4);
      else {
        const size_t n = root.size() + 5 + scratch.size();
        return root + "/[..]" +
               scratch.substr(n - max_length, max_length - root.size() - 5);
      }
    }
  }
}

}  // namespace glight::theatre::folders

#endif
