#include "jsonwriter.h"

#include <codecvt>
#include <cuchar>
#include <locale>
#include <vector>

namespace glight::json {

std::string JsonWriter::Encode(const std::string_view& str) {
  std::vector<char32_t> result{U'\"'};
  std::u32string utf32 =
      std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(
          str.begin(), str.end());
  for (char32_t c : utf32) {
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
  return std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.to_bytes(
             result.data(), result.data() + result.size()) +
         '\"';
}

}  // namespace glight::json
