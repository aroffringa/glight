#include "../system/jsonreader.h"

#include <boost/test/unit_test.hpp>

#include <sstream>

namespace {
void Check(const char* input, bool expected_result,
           const char* output = nullptr) {
  std::istringstream stream(input + 1);
  std::string output_string;
  const bool result =
      json::details::ReadNumber(input[0], stream, output_string);
  BOOST_TEST_INFO("input=\"" << input << "\"");
  BOOST_TEST(result == expected_result);
  if (result && expected_result) {
    BOOST_CHECK_EQUAL(output_string, output);
  }
}

void SCheck(const char* input, bool expected_result, const char* output) {
  std::istringstream stream(input);
  std::string output_string;
  const bool result = json::details::ReadString(stream, output_string);
  BOOST_TEST_INFO("input=\"" << input << "\", output=\"" << output_string
                             << "\"");
  BOOST_TEST(result == expected_result);
  BOOST_CHECK_EQUAL(output_string, output);
}

std::unique_ptr<json::Node> Parse(const char* input) {
  std::istringstream str(input);
  return json::Parse(str);
}

}  // namespace

BOOST_AUTO_TEST_SUITE(json_reader)

BOOST_AUTO_TEST_CASE(read_number) {
  Check("0", true, "0");
  Check("9", true, "9");
  Check("0 ", true, "0");
  Check("9\t", true, "9");
  Check("123\n", true, "123");
  Check("-", false);
  Check("+", false);
  Check("E", false);
  Check("0.", false);
  Check("0E", false);
  Check("0.E", false);
  Check("0.1E", false);
  Check("01", false);
  Check("1.", false);
  Check("0.3E+", false);
  Check("-3e -12 ", false);
  Check("10?", true, "10");
  Check("19", true, "19");
  Check("700!", true, "700");
  Check("2.7", true, "2.7");
  Check("3.1415926535 ", true, "3.1415926535");
  Check("+0.14 ", true, "+0.14");
  Check("1e8 ", true, "1e8");
  Check("-0E8 ", true, "-0E8");
  Check("2.99792458e+8 ", true, "2.99792458e+8");
  Check("-3e-12 ", true, "-3e-12");
}

BOOST_AUTO_TEST_CASE(read_string) {
  SCheck("", false, "");
  SCheck("Missing close", false, "Missing close");
  SCheck("\"", true, "");
  SCheck("Hi!\"\t", true, "Hi!");
  SCheck("This has spaces\" ", true, "This has spaces");
  SCheck("\\\" <- quotes\"", true, "\" <- quotes");
  SCheck("Line 1\\nNewline\"\n", true, "Line 1\nNewline");
  SCheck("Unicode: — André\"}", true, "Unicode: — André");
  SCheck("Escapes:\\\"\\n\\r\\b\\\\\\/\\f\\tend.\"]", true,
         "Escapes:\"\n\r\b\\/\f\tend.");
  SCheck("This: \\x wrong escape", false, "This: ");
}

BOOST_AUTO_TEST_CASE(null) {
  std::unique_ptr<json::Node> node = Parse("null");
  BOOST_CHECK(dynamic_cast<json::Null*>(node.get()));
}

BOOST_AUTO_TEST_CASE(boolean) {
  std::unique_ptr<json::Node> node = Parse("true");
  json::Boolean* b = dynamic_cast<json::Boolean*>(node.get());
  BOOST_REQUIRE(b);
  BOOST_CHECK_EQUAL(b->value, true);

  node = Parse("false");
  b = dynamic_cast<json::Boolean*>(node.get());
  BOOST_REQUIRE(b);
  BOOST_CHECK_EQUAL(b->value, false);
}

BOOST_AUTO_TEST_CASE(number) {
  std::unique_ptr<json::Node> node = Parse("3.1415e-0");
  json::Number* n = dynamic_cast<json::Number*>(node.get());
  BOOST_REQUIRE(n);
  BOOST_CHECK_EQUAL(n->value, "3.1415e-0");
}

BOOST_AUTO_TEST_CASE(string) {
  std::unique_ptr<json::Node> node = Parse("\"Simple\"");
  json::String* s = dynamic_cast<json::String*>(node.get());
  BOOST_REQUIRE(s);
  BOOST_CHECK_EQUAL(s->value, "Simple");

  node = Parse("\"\\\"hello world\\\"\\n— André\"");
  s = dynamic_cast<json::String*>(node.get());
  BOOST_REQUIRE(s);
  BOOST_CHECK_EQUAL(s->value, "\"hello world\"\n— André");
}

BOOST_AUTO_TEST_CASE(object) {
  std::unique_ptr<json::Node> node =
      Parse("{ \"name\": \"André\", \"person\": true, \"age\": 40 }");
  json::Object* o = dynamic_cast<json::Object*>(node.get());
  BOOST_REQUIRE(o);
  BOOST_CHECK_EQUAL(o->children.size(), 3);
  const json::String* name =
      dynamic_cast<json::String*>(o->children["name"].get());
  BOOST_REQUIRE(name);
  BOOST_CHECK_EQUAL(name->value, "André");
  const json::Boolean* person =
      dynamic_cast<json::Boolean*>(o->children["person"].get());
  BOOST_REQUIRE(person);
  BOOST_CHECK_EQUAL(person->value, true);
  const json::Number* age =
      dynamic_cast<json::Number*>(o->children["age"].get());
  BOOST_REQUIRE(age);
  BOOST_CHECK_EQUAL(age->value, "40");
}

BOOST_AUTO_TEST_CASE(array) {
  std::unique_ptr<json::Node> node = Parse("[\"André\",false,40]");
  const json::Array* a = dynamic_cast<json::Array*>(node.get());
  BOOST_REQUIRE(a);
  BOOST_REQUIRE_EQUAL(a->items.size(), 3);
  const json::String* andre = dynamic_cast<json::String*>(a->items[0].get());
  BOOST_REQUIRE(andre);
  BOOST_CHECK_EQUAL(andre->value, "André");
  const json::Boolean* boolean =
      dynamic_cast<json::Boolean*>(a->items[1].get());
  BOOST_REQUIRE(boolean);
  BOOST_CHECK_EQUAL(boolean->value, false);
  const json::Number* fourty = dynamic_cast<json::Number*>(a->items[2].get());
  BOOST_REQUIRE(fourty);
  BOOST_CHECK_EQUAL(fourty->value, "40");
}

BOOST_AUTO_TEST_CASE(recursive) {
  std::unique_ptr<json::Node> node = Parse(R"({
  "person": {"name": "Xam", "age": 3.5},
  "relatives": [{"name": "André"}, {"name": "Lars"}, {"name": "Shanthi"}]
})");
  json::Object* root = dynamic_cast<json::Object*>(node.get());
  BOOST_REQUIRE(root);
  BOOST_CHECK_EQUAL(root->children.size(), 2);

  json::Object* person =
      dynamic_cast<json::Object*>(root->children["person"].get());
  BOOST_REQUIRE(person);
  BOOST_CHECK_EQUAL(person->children.size(), 2);
  const json::String* name =
      dynamic_cast<json::String*>(person->children["name"].get());
  BOOST_REQUIRE(name);
  BOOST_CHECK_EQUAL(name->value, "Xam");
  const json::Number* age =
      dynamic_cast<json::Number*>(person->children["age"].get());
  BOOST_REQUIRE(age);
  BOOST_CHECK_EQUAL(age->value, "3.5");

  json::Array* relatives =
      dynamic_cast<json::Array*>(root->children["relatives"].get());
  BOOST_REQUIRE(relatives);
  BOOST_REQUIRE_EQUAL(relatives->items.size(), 3);

  json::Object* andre = dynamic_cast<json::Object*>(relatives->items[0].get());
  BOOST_REQUIRE(andre);
  BOOST_REQUIRE_EQUAL(andre->children.size(), 1);
  json::String* a_name =
      dynamic_cast<json::String*>(andre->children["name"].get());
  BOOST_REQUIRE(a_name);
  BOOST_REQUIRE_EQUAL(a_name->value, "André");

  json::Object* lars = dynamic_cast<json::Object*>(relatives->items[1].get());
  BOOST_REQUIRE(lars);
  BOOST_REQUIRE_EQUAL(lars->children.size(), 1);
  json::String* l_name =
      dynamic_cast<json::String*>(lars->children["name"].get());
  BOOST_REQUIRE(l_name);
  BOOST_REQUIRE_EQUAL(l_name->value, "Lars");

  json::Object* shanthi =
      dynamic_cast<json::Object*>(relatives->items[2].get());
  BOOST_REQUIRE(shanthi);
  BOOST_REQUIRE_EQUAL(shanthi->children.size(), 1);
  json::String* s_name =
      dynamic_cast<json::String*>(shanthi->children["name"].get());
  BOOST_REQUIRE(s_name);
  BOOST_REQUIRE_EQUAL(s_name->value, "Shanthi");
}

BOOST_AUTO_TEST_SUITE_END()
