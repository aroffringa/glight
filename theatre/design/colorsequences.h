#ifndef GLIGHT_THEATRE_DESIGN_COLOR_SEQUENCES_H_
#define GLIGHT_THEATRE_DESIGN_COLOR_SEQUENCES_H_

#include <string>
#include <vector>

#include "../color.h"

namespace glight::theatre {

std::vector<std::string> GetDefaultColorSequences() {
  return {"Primary", "Fire", "Pastel", "Rainbow", "Sea"};
}

std::vector<std::pair<unsigned, Color>> GetDefaultColorSequence(
    const std::string& name) {
  if (name == "Primary") {
    return {{7, Color(255, 128, 0)},   {5, Color(0, 255, 255)},
            {0, Color(255, 0, 0)},     {1, Color(0, 255, 0)},
            {6, Color(255, 255, 255)}, {2, Color(0, 0, 255)},
            {3, Color(255, 255, 0)},   {4, Color(255, 0, 255)}};
  } else if (name == "Fire") {
    return {{0, Color(255, 32, 0)},    {1, Color(255, 192, 0)},
            {11, Color(255, 128, 64)}, {2, Color(255, 0, 0)},
            {7, Color(255, 96, 0)},    {5, Color(255, 0, 32)},
            {3, Color(255, 128, 0)},   {4, Color(255, 32, 0)},
            {6, Color(255, 255, 0)},   {8, Color(255, 0, 0)},
            {9, Color(255, 192, 0)},   {10, Color(255, 32, 32)}};
  } else if (name == "Sea") {
    return {{0, Color(0, 0, 255)},   {1, Color(0, 255, 128)},
            {2, Color(0, 128, 255)}, {3, Color(0, 255, 255)},
            {5, Color(0, 0, 255)},   {4, Color(0, 255, 128)},
            {6, Color(0, 64, 255)},  {7, Color(0, 255, 192)}};
  } else if (name == "Pastel") {
    return {{6, Color(255, 192, 128)}, {5, Color(128, 255, 255)},
            {0, Color(255, 128, 128)}, {1, Color(128, 255, 128)},
            {2, Color(128, 128, 255)}, {3, Color(255, 255, 128)},
            {4, Color(255, 128, 255)}};
  } else if (name == "Rainbow") {
    return {{0, Color(255, 0, 0)},    {7, Color(255, 83, 0)},
            {4, Color(255, 165, 0)},  {12, Color(255, 210, 0)},
            {3, Color(255, 255, 0)},  {9, Color(128, 255, 0)},
            {1, Color(0, 255, 0)},    {11, Color(0, 255, 255)},
            {2, Color(0, 0, 255)},    {10, Color(74, 0, 255)},
            {5, Color(148, 0, 255)},  {8, Color(196, 70, 255)},
            {6, Color(255, 139, 255)}};
  }
  return {};
}

std::vector<Color> GetDefaultColorSequence(const std::string& name,
                                           size_t size) {
  const std::vector<std::pair<unsigned, Color>> sequence =
      GetDefaultColorSequence(name);
  std::vector<Color> result;
  if (!sequence.empty()) {
    result.reserve(size);
    while (result.size() < size) {
      for (const std::pair<unsigned, Color>& entry : sequence) {
        if (entry.first < size) result.emplace_back(entry.second);
      }
    }
    result.resize(size, Color(0, 0, 0));
  }
  return result;
}

}  // namespace glight::theatre

#endif
