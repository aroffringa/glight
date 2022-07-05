#ifndef JSON_WRITER_H_
#define JSON_WRITER_H_

#include <iostream>
#include <string>
#include <string_view>

class JsonWriter {
 public:
  JsonWriter() = default;

  JsonWriter(std::ostream& stream) : stream_(&stream) {}

  void Name(const char* name) {
    Next();
    Out() << Encode(name) << ": ";
    state_ = State::AfterName;
  }

  void StartObject() { Start('{'); }

  void StartObject(const char* name) {
    Name(name);
    StartObject();
  }

  void EndObject() { End('}'); }

  void StartArray() { Start('['); }

  void StartArray(const char* name) {
    Name(name);
    StartArray();
  }

  void EndArray() { End(']'); }

  void Boolean(const char* name, bool value) {
    Name(name);
    Boolean(value);
  }

  void Boolean(bool value) {
    Next();
    Out() << (value ? "true" : "false");
    state_ = State::AfterItem;
  }

  void Number(const char* name, double number) {
    Name(name);
    Number(number);
  }

  void Number(double number) { NumberGeneric(number); }

  void Number(int number) { NumberGeneric(number); }

  void Number(size_t number) { NumberGeneric(number); }

  void String(const std::string& str) {
    Next();
    Out() << Encode(std::string_view(str));
    state_ = State::AfterItem;
  }

  void String(const char* name, const std::string& str) {
    Name(name);
    String(str);
  }

  void Null(const char* name) {
    Name(name);
    Null();
  }

  void Null() {
    Next();
    Out() << "null";
    state_ = State::AfterItem;
  }

  static std::string Encode(const std::string_view& str);
  static std::string Encode(const std::string& str) {
    return Encode(std::string_view(str));
  }
  static std::string Encode(const char* str) {
    return Encode(std::string_view(str));
  }

 private:
  void Start(const char c) {
    Next();
    Out() << c;
    ++indent_;
    state_ = State::AfterStart;
  }

  void End(const char c) {
    if (indent_ == 0) throw std::runtime_error("Unmatched end() call");
    --indent_;
    if (state_ != State::AfterStart) {
      Out() << '\n';
      Indent();
    }
    Out() << c;
    state_ = State::AfterItem;
  }

  void Next() {
    switch (state_) {
      case State::NewLine:
      case State::Empty:
      case State::AfterName:
        break;
      case State::AfterStart:
        Out() << "\n";
        Indent();
        break;
      case State::AfterItem:
        Out() << ",\n";
        Indent();
        break;
    }
    state_ = State::NewLine;
  }

  void Indent() {
    for (size_t i = 0; i != indent_; ++i) Out() << "  ";
  }

  std::ostream& Out() { return *stream_; }

  template <typename T>
  void NumberGeneric(T number) {
    Next();
    Out() << number;
    state_ = State::AfterItem;
  }

  std::ostream* stream_ = nullptr;

  enum class State {
    Empty,
    AfterName,
    AfterStart,
    AfterItem,
    NewLine
  } state_ = State::Empty;

  size_t indent_ = 0;
};

#endif
