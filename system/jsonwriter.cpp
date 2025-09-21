#include "jsonwriter.h"

#include <cuchar>
#include <vector>

namespace glight::json {

JsonWriter::JsonWriter(std::ostream& stream) : stream_(&stream) {}

std::string JsonWriter::Encode(const std::string_view& str) {
  std::u32string u32str(str.begin(), str.end());
  std::vector<char32_t> result;
  result.emplace_back('\"');
  for (char32_t c : u32str) {
    switch (c) {
      case U'\\':
      case U'\"':
      case U'/':
        result.emplace_back(U'\\');
        result.emplace_back(c);
        break;
      case U'\b':
        result.emplace_back(U'\\');
        result.emplace_back(U'b');
        break;
      case U'\f':
        result.emplace_back(U'\\');
        result.emplace_back(U'f');
        break;
      case U'\n':
        result.emplace_back(U'\\');
        result.emplace_back(U'n');
        break;
      case U'\r':
        result.emplace_back(U'\\');
        result.emplace_back(U'r');
        break;
      case U'\t':
        result.emplace_back(U'\\');
        result.emplace_back(U't');
        break;
      default:
        result.emplace_back(c);
        break;
    }
  }
  result.emplace_back(U'\"');
  return std::string(result.begin(), result.end());
}

}  // namespace glight::json
