#ifndef GLIGHT_TESTS_THEATRE_FILTERS_H_
#define GLIGHT_TESTS_THEATRE_FILTERS_H_

#include "theatre/fixturemodefunction.h"

namespace glight::theatre {

inline std::vector<FixtureModeFunction> MakeFunctionList(
    const std::vector<FunctionType>& types) {
  std::vector<FixtureModeFunction> list;
  for (size_t i = 0; i != types.size(); ++i) {
    list.emplace_back(FixtureModeFunction(types[i], i, {}, 0));
  }
  return list;
}

inline std::vector<FixtureModeFunction> GetWhiteFunctionExample() {
  return MakeFunctionList({FunctionType::White});
}

inline std::vector<FixtureModeFunction> GetRGBMFunctionsExample() {
  return MakeFunctionList({FunctionType::Red, FunctionType::Green,
                           FunctionType::Blue, FunctionType::Master});
}

inline std::vector<FixtureModeFunction> GetRGBMSFunctionsExample() {
  return MakeFunctionList({FunctionType::Red, FunctionType::Green,
                           FunctionType::Blue, FunctionType::Master,
                           FunctionType::Strobe});
}

inline std::vector<FixtureModeFunction> GetMRGBSFunctionsExample() {
  return MakeFunctionList({FunctionType::Master, FunctionType::Red,
                           FunctionType::Green, FunctionType::Blue,
                           FunctionType::Strobe});
}

}  // namespace glight::theatre

#endif
