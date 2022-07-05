#ifndef JSON_READER_H_
#define JSON_READER_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace json {

struct Node {
  virtual ~Node() { }
};
  
struct Object : public Node {
  std::map<std::string, std::unique_ptr<Node>> children;
};

struct Array : public Node {
  std::vector<std::unique_ptr<Node>> items;
};

struct Null : public Node {
};

struct String : public Node {
  String(std::string v) : value(std::move(v)) { }
  std::string value;
};

struct Boolean : public Node {
  Boolean(bool v) : value(v) { }
  bool value;
};

struct Number : public Node {
  Number(std::string v) : value(std::move(v)) { }
  // A number is saved as string so that parsing is delayed until
  // the type of the destination is known
  std::string value;
};

namespace details {
bool ReadNumber(char first, std::istream& stream, std::string& data);
bool ReadString(std::istream& stream, std::string& data);
}

std::unique_ptr<Node> Parse(std::istream& stream);

}

#endif
