#include "jsonreader.h"

#include <sstream>

namespace glight::json {
namespace {

std::unique_ptr<Node> ParseValue(std::istream& stream);
std::unique_ptr<Node> ParseValue(std::istream& stream, const char token,
                                 const std::string& data);

void SkipWhitespace(std::istream& stream) {
  std::istream::int_type c = stream.peek();
  while (c == '\n' || c == '\r' || c == ' ' || c == '\t') {
    char ignored;
    stream.read(&ignored, 1);
    c = stream.peek();
  }
}

char NextToken(std::istream& stream, std::string& data) {
  SkipWhitespace(stream);
  char c;
  stream.read(&c, 1);
  if (!stream) {
    data = std::string();
    return 0;
  }
  if (c == '{' || c == '[' || c == '}' || c == ']' || c == ':' || c == ',')
    return c;
  else if (c == '"') {
    if (details::ReadString(stream, data))
      return '"';
    else
      return 0;
  } else if (c == '-' || c == '+' || (c >= '0' && c <= '9')) {
    if (details::ReadNumber(c, stream, data))
      return 'N';
    else
      return 0;
  } else if (c == 'n') {
    char ull[3];
    stream.read(ull, 3);
    if (!stream) {
      data = std::string(1, c);
      return 0;
    }
    if (ull[0] != 'u' || ull[1] != 'l' || ull[2] != 'l') {
      data = std::string(1, c) + ull[0] + ull[1] + ull[2];
      return 0;
    }
    return c;
  } else if (c == 't') {
    char rue[3];
    stream.read(rue, 3);
    if (!stream) {
      data = std::string(1, c);
      return 0;
    }
    if (rue[0] != 'r' || rue[1] != 'u' || rue[2] != 'e') {
      data = std::string(1, c) + rue[0] + rue[1] + rue[2];
      return 0;
    }
    return c;
  } else if (c == 'f') {
    char alse[4];
    stream.read(alse, 4);
    if (!stream) {
      data = std::string(1, c);
      return 0;
    }
    if (alse[0] != 'a' || alse[1] != 'l' || alse[2] != 's' || alse[3] != 'e') {
      data = std::string(1, c) + alse[0] + alse[1] + alse[2] + alse[3];
      return 0;
    }
    return c;
  } else {
    data = std::string(1, c);
    return 0;
  }
}

std::unique_ptr<Node> ParseObject(std::istream& stream) {
  std::unique_ptr<Object> result = std::make_unique<Object>();
  std::string name;
  char t = NextToken(stream, name);
  while (t != '}') {
    if (t == '"') {
      std::string scratch;
      const char colon = NextToken(stream, scratch);
      if (colon != ':')
        throw std::runtime_error(
            std::string("Parse error, expecting colon, got ") + colon);
      result->children.emplace(std::move(name), ParseValue(stream));
      t = NextToken(stream, name);
      if (t == ',') {
        t = NextToken(stream, name);
        if (t == '}')
          throw std::runtime_error("Extra trailing comma in object");
      }
    } else {
      throw std::runtime_error("Expecting name or '}' in object");
    }
  }
  return result;
}

std::unique_ptr<Node> ParseArray(std::istream& stream) {
  std::unique_ptr<Array> result = std::make_unique<Array>();
  std::string data;
  char token = NextToken(stream, data);
  while (token != ']') {
    result->items.emplace_back(ParseValue(stream, token, data));
    token = NextToken(stream, data);
    if (token == ',') {
      token = NextToken(stream, data);
      if (token == ']')
        throw std::runtime_error("Extra trailing comma in array");
    }
  }
  return result;
}

std::unique_ptr<Node> ParseValue(std::istream& stream, const char token,
                                 const std::string& data) {
  switch (token) {
    case '{':
      return ParseObject(stream);
    case '[':
      return ParseArray(stream);
    case 'n':
      return std::make_unique<Null>();
    case 't':
      return std::make_unique<Boolean>(true);
    case 'f':
      return std::make_unique<Boolean>(false);
    case '"':
      return std::make_unique<String>(data);
    case 'N':
      return std::make_unique<Number>(data);
    default:
      throw std::runtime_error("Parse error, unexpected token: '" +
                               (token + data) + "'");
  }
}

std::unique_ptr<Node> ParseValue(std::istream& stream) {
  std::string data;
  const char token = NextToken(stream, data);
  return ParseValue(stream, token, data);
}

}  // namespace

namespace details {
bool ReadString(std::istream& stream, std::string& data) {
  std::ostringstream str;
  char c;
  do {
    stream.read(&c, 1);
    if (!stream) {
      data = str.str();
      return false;
    }
    if (c == '\\') {
      stream.read(&c, 1);
      if (!stream) {
        data = str.str();
        return false;
      }
      switch (c) {
        case '"':
          str << '"';
          break;
        case '\\':
          str << '\\';
          break;
        case '/':
          str << '/';
          break;
        case 'b':
          str << '\b';
          break;
        case 'f':
          str << '\f';
          break;
        case 'n':
          str << '\n';
          break;
        case 'r':
          str << '\r';
          break;
        case 't':
          str << '\t';
          break;
        default:
          data = str.str();
          return false;
      }
      c = 0;
    } else if (c != '"')
      str << c;
  } while (c != '"');
  data = str.str();
  return true;
}

bool ReadNumber(char first, std::istream& stream, std::string& data) {
  enum class State {
    AfterSign,
    AfterZero,
    AfterOneNine,
    AfterFraction,
    InFractionDigits,
    AfterExp,
    AfterExpSign,
    InExpDigits,
    End
  } state;
  std::ostringstream str;
  str << first;
  if (first == '0')
    state = State::AfterZero;
  else if (first >= '1' && first <= '9')
    state = State::AfterOneNine;
  else if (first == '-' || first == '+')
    state = State::AfterSign;
  else
    return false;

  std::istream::int_type c;
  do {
    c = stream.peek();
    if (c == '0') {
      switch (state) {
        case State::AfterOneNine:
        case State::InFractionDigits:
        case State::InExpDigits:
          break;
        case State::AfterSign:
          state = State::AfterZero;
          break;
        case State::End:
        case State::AfterZero:
          return false;  // Two leading zeros are not allowed
        case State::AfterFraction:
          state = State::InFractionDigits;
          break;
        case State::AfterExp:
        case State::AfterExpSign:
          state = State::InExpDigits;
          break;
      }
    } else if (c >= '1' && c <= '9') {
      switch (state) {
        case State::AfterSign:
          state = State::AfterOneNine;
          break;
        case State::AfterOneNine:
        case State::InFractionDigits:
        case State::InExpDigits:
          break;
        case State::End:
        case State::AfterZero:
          return false;  // Multi-digit number may not lead with zeros
        case State::AfterFraction:
          state = State::InFractionDigits;
          break;
        case State::AfterExp:
        case State::AfterExpSign:
          state = State::InExpDigits;
          break;
      }
    } else if (c == '-' || c == '+') {
      switch (state) {
        case State::AfterZero:
        case State::AfterOneNine:
        case State::InFractionDigits:
        case State::AfterSign:
        case State::AfterFraction:
        case State::AfterExpSign:
        case State::InExpDigits:
        case State::End:
          return false;
        case State::AfterExp:
          state = State::AfterExpSign;
          break;
      }
    } else if (c == '.') {
      switch (state) {
        case State::AfterZero:
        case State::AfterOneNine:
          state = State::AfterFraction;
          break;
        case State::InFractionDigits:
        case State::AfterSign:
        case State::AfterFraction:
        case State::AfterExp:
        case State::AfterExpSign:
        case State::InExpDigits:
        case State::End:
          return false;
      }
    } else if (c == 'E' || c == 'e') {
      switch (state) {
        case State::AfterZero:
        case State::AfterOneNine:
        case State::InFractionDigits:
          state = State::AfterExp;
          break;
        case State::AfterSign:
        case State::AfterFraction:
        case State::AfterExp:
        case State::AfterExpSign:
        case State::InExpDigits:
        case State::End:
          return false;
      }
    } else {  // includes eof and 0
      switch (state) {
        case State::AfterZero:
        case State::AfterOneNine:
        case State::InFractionDigits:
        case State::InExpDigits:
          break;
        case State::AfterFraction:
        case State::AfterSign:
        case State::AfterExp:
        case State::AfterExpSign:
        case State::End:
          return false;
      }
      state = State::End;
    }
    if (state != State::End) {
      str << static_cast<char>(stream.get());
    }
  } while (state != State::End);
  data = str.str();
  return true;
}
}  // namespace details

std::unique_ptr<Node> Parse(std::istream& stream) { return ParseValue(stream); }

}  // namespace json
