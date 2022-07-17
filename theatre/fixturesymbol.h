#ifndef THEATRE_FIXTURE_SYMBOL_H
#define THEATRE_FIXTURE_SYMBOL_H

#include <stdexcept>
#include <string>
#include <vector>

namespace glight::theatre {

class FixtureSymbol {
 public:
  enum Symbol { Hidden, Small, Normal, Large };

  FixtureSymbol(Symbol symbol = Normal) : _symbol(symbol) {}

  explicit FixtureSymbol(const std::string &name)
      : _symbol(nameToSymbol(name)) {}

  std::string Name() const { return symbolToName(_symbol); }

  Symbol Value() const { return _symbol; }

  static std::vector<Symbol> List() {
    return std::vector<Symbol>{Hidden, Small, Normal, Large};
  }

 private:
  static Symbol nameToSymbol(const std::string &name) {
    if (name == "hidden")
      return Hidden;
    else if (name == "small")
      return Small;
    else if (name == "normal")
      return Normal;
    else if (name == "large")
      return Large;
    throw std::runtime_error("Symbol unknown: " + name);
  }

  static std::string symbolToName(Symbol symbol) {
    switch (symbol) {
      case Hidden:
        return "hidden";
      case Small:
        return "small";
      case Normal:
        return "normal";
      case Large:
        return "large";
    }
    return "normal";
  }

  Symbol _symbol;
};

inline bool operator==(const FixtureSymbol &lhs, FixtureSymbol::Symbol rhs) {
  return lhs.Value() == rhs;
}
inline bool operator==(FixtureSymbol::Symbol lhs, const FixtureSymbol &rhs) {
  return lhs == rhs.Value();
}
inline bool operator!=(const FixtureSymbol &lhs, FixtureSymbol::Symbol rhs) {
  return lhs.Value() != rhs;
}
inline bool operator!=(FixtureSymbol::Symbol lhs, const FixtureSymbol &rhs) {
  return lhs != rhs.Value();
}

}  // namespace glight::theatre

#endif
