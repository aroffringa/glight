#include "../system/jsonwriter.h"

#include <boost/test/unit_test.hpp>

#include <sstream>

using glight::json::JsonWriter;

BOOST_AUTO_TEST_SUITE(json_writer)

BOOST_AUTO_TEST_CASE(object) {
  std::ostringstream s;
  JsonWriter writer(s);
  writer.Name("birth_date");
  writer.StartObject();
  writer.Name("day");
  writer.StartObject();
  writer.EndObject();
  writer.Name("month");
  writer.StartObject();
  writer.EndObject();
  writer.Name("year");
  writer.StartObject();
  writer.EndObject();
  writer.EndObject();

  BOOST_CHECK_EQUAL(s.str(), R"("birth_date": {
  "day": {},
  "month": {},
  "year": {}
})");
}

BOOST_AUTO_TEST_CASE(boolean) {
  std::ostringstream s;
  JsonWriter writer(s);
  writer.Name("is_positive");
  writer.Boolean(true);
  writer.Name("is_negative");
  writer.Boolean(false);
  BOOST_CHECK_EQUAL(s.str(), R"("is_positive": true,
"is_negative": false)");
}

BOOST_AUTO_TEST_CASE(null) {
  std::ostringstream s;
  JsonWriter writer(s);
  writer.Name("first");
  writer.Null();
  writer.Name("second");
  writer.Null();
  BOOST_CHECK_EQUAL(s.str(), R"("first": null,
"second": null)");
}

BOOST_AUTO_TEST_CASE(array) {
  std::ostringstream s;
  JsonWriter writer(s);
  writer.Name("list");
  writer.StartArray();
  writer.Null();
  writer.Boolean(false);
  writer.Boolean(true);
  writer.EndArray();
  BOOST_CHECK_EQUAL(s.str(), R"("list": [
  null,
  false,
  true
])");
}

BOOST_AUTO_TEST_CASE(object_array) {
  std::ostringstream s;
  JsonWriter writer(s);
  writer.StartArray("list");
  writer.StartObject();
  writer.Null("a");
  writer.EndObject();
  writer.StartObject();
  writer.Boolean("b", false);
  writer.Boolean("c", true);
  writer.EndObject();
  writer.EndArray();
  BOOST_CHECK_EQUAL(s.str(), R"("list": [
  {
    "a": null
  },
  {
    "b": false,
    "c": true
  }
])");
}

BOOST_AUTO_TEST_CASE(number) {
  std::ostringstream s;
  JsonWriter writer(s);
  writer.Name("date");
  writer.StartObject();
  writer.Name("year");
  writer.Number(1982);
  writer.Name("month");
  writer.Number(5);
  writer.Name("day");
  writer.Number(8.3);
  writer.EndObject();
  BOOST_CHECK_EQUAL(s.str(), R"("date": {
  "year": 1982,
  "month": 5,
  "day": 8.3
})");
}

BOOST_AUTO_TEST_CASE(stringval) {
  std::ostringstream s;
  JsonWriter writer(s);
  writer.Name("name");
  writer.String("André");
  writer.Name("quote");
  writer.String("\"");
  BOOST_CHECK_EQUAL(s.str(), R"("name": "André",
"quote": "\"")");
}

BOOST_AUTO_TEST_CASE(encode) {
  std::ostringstream s;
  BOOST_CHECK_EQUAL(JsonWriter::Encode("André"), "\"André\"");
  BOOST_CHECK_EQUAL(JsonWriter::Encode("\"\'\\\r\n\f\b\t"),
                    "\"\\\"\'\\\\\\r\\n\\f\\b\\t\"");
}

BOOST_AUTO_TEST_SUITE_END()
