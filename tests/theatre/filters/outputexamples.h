#ifndef GLIGHT_TESTS_THEATRE_FILTERS_H_
#define GLIGHT_TESTS_THEATRE_FILTERS_H_

#include "theatre/fixturetypefunction.h"

namespace glight::theatre {

inline std::vector<FixtureTypeFunction> MakeFunctionList(
    const std::vector<FunctionType>& types) {
  std::vector<FixtureTypeFunction> list;
  for (size_t i = 0; i != types.size(); ++i) {
    list.emplace_back(FixtureTypeFunction(types[i], i, {}, 0));
  }
  return list;
}

inline std::vector<FixtureTypeFunction> GetWhiteFunctionExample() {
  return MakeFunctionList({FunctionType::White});
}

inline std::vector<FixtureTypeFunction> GetRGBMFunctionsExample() {
  return MakeFunctionList({FunctionType::Red, FunctionType::Green,
                           FunctionType::Blue, FunctionType::Master});
}

inline std::vector<FixtureTypeFunction> GetRGBMSFunctionsExample() {
  return MakeFunctionList({FunctionType::Red, FunctionType::Green,
                           FunctionType::Blue, FunctionType::Master,
                           FunctionType::Strobe});
}

inline std::vector<FixtureTypeFunction> GetMRGBSFunctionsExample() {
  return MakeFunctionList({FunctionType::Master, FunctionType::Red,
                           FunctionType::Green, FunctionType::Blue,
                           FunctionType::Strobe});
}

}  // namespace glight::theatre

#endif
