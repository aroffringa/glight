#ifndef JSON_READER_H_
#define JSON_READER_H_

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "dereferencing_iterator.h"

namespace glight::json {

struct Node {
  virtual ~Node() {}
};

struct Object : public Node {
  std::map<std::string, std::unique_ptr<Node>> children;
  using iterator = system::DereferencingMapIterator<
      std::map<std::string, std::unique_ptr<Node>>::iterator>;
  using const_iterator = system::DereferencingMapIterator<
      std::map<std::string, std::unique_ptr<Node>>::const_iterator>;

  const Node &operator[](const char *name) const {
    const auto iter = children.find(name);
    if (iter == children.end())
      throw std::runtime_error(std::string("Missing field in json file: ") +
                               name);
    else
      return *iter->second;
  }

  bool contains(const char *name) const {
    return children.find(name) != children.end();
  }

  const_iterator find(const char *name) const {
    return const_iterator(children.find(name));
  }

  const_iterator begin() const { return const_iterator(children.begin()); }

  const_iterator end() const { return const_iterator(children.end()); }
};

struct Array : public Node {
  std::vector<std::unique_ptr<Node>> items;

  using iterator = system::DereferencingIterator<
      std::vector<std::unique_ptr<Node>>::iterator>;
  using const_iterator = system::DereferencingIterator<
      std::vector<std::unique_ptr<Node>>::const_iterator>;

  iterator begin() { return iterator(items.begin()); }
  const_iterator begin() const { return const_iterator(items.begin()); }
  iterator end() { return iterator(items.end()); }
  const_iterator end() const { return const_iterator(items.end()); }
};

struct Null : public Node {};

struct String : public Node {
  String(std::string v) : value(std::move(v)) {}
  std::string value;
};

struct Boolean : public Node {
  Boolean(bool v) : value(v) {}
  bool value;
};

struct Number : public Node {
  Number(std::string v) : value(std::move(v)) {}
  // A number is saved as string so that parsing is delayed until
  // the type of the destination is known
  std::string value;

  size_t AsSize() const { return AsType<size_t>(); }
  int AsInt() const { return AsType<int>(); }
  unsigned char AsUChar() const { return AsType<unsigned>(); }
  unsigned AsUInt() const { return AsType<unsigned>(); }
  float AsFloat() const { return AsType<float>(); }
  double AsDouble() const { return AsType<double>(); }

 private:
  template <typename T>
  T AsType() const {
    std::istringstream s(value);
    T v;
    s >> v;
    return v;
  }
};

inline const Array &ToArr(const Node &node) {
  return dynamic_cast<const Array &>(node);
}

inline bool ToBool(const Node &node) {
  return dynamic_cast<const Boolean &>(node).value;
}

inline bool OptionalBool(const Object &parent, const char *name,
                         bool default_value) {
  const Object::const_iterator iter = parent.find(name);
  return iter == parent.end() ? default_value
                              : dynamic_cast<const Boolean &>(*iter).value;
}

inline const Number &ToNum(const Node &node) {
  return dynamic_cast<const Number &>(node);
}

inline size_t OptionalSize(const Object &parent, const char *name,
                           size_t default_value) {
  const Object::const_iterator iter = parent.find(name);
  return iter == parent.end() ? default_value : ToNum(*iter).AsSize();
}

inline unsigned OptionalUInt(const Object &parent, const char *name,
                             unsigned default_value) {
  const Object::const_iterator iter = parent.find(name);
  return iter == parent.end() ? default_value : ToNum(*iter).AsUInt();
}

inline double OptionalDouble(const Object &parent, const char *name,
                             double default_value) {
  const Object::const_iterator iter = parent.find(name);
  return iter == parent.end() ? default_value : ToNum(*iter).AsDouble();
}

inline const Object &ToObj(const Node &node) {
  return dynamic_cast<const Object &>(node);
}
inline const std::string &ToStr(const Node &node) {
  return dynamic_cast<const String &>(node).value;
}

inline std::string OptionalString(const Object &parent, const char *name,
                                  const std::string &default_value) {
  const Object::const_iterator iter = parent.find(name);
  return iter == parent.end() ? default_value : ToStr(*iter);
}

namespace details {
bool ReadNumber(char first, std::istream &stream, std::string &data);
bool ReadString(std::istream &stream, std::string &data);
}  // namespace details

std::unique_ptr<Node> Parse(std::istream &stream);

}  // namespace glight::json

#endif
