#include "../theatre/folderoperations.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using glight::theatre::folders::Depth;
using glight::theatre::folders::LastName;
using glight::theatre::folders::ParentPath;
using glight::theatre::folders::RemoveRoot;
using glight::theatre::folders::Root;
using glight::theatre::folders::ShortDescription;

BOOST_AUTO_TEST_SUITE(folder_operations)

BOOST_AUTO_TEST_CASE(parent_path) {
  BOOST_CHECK_EQUAL("", ParentPath(""));
  BOOST_CHECK_EQUAL("", ParentPath("root"));
  BOOST_CHECK_EQUAL("root", ParentPath("root/file"));
  BOOST_CHECK_EQUAL("a/b", ParentPath("a/b/c"));
  BOOST_CHECK_EQUAL("a/a and b", ParentPath("a/a and b/c"));
  BOOST_CHECK_EQUAL("1/2/3", ParentPath("1/2/3/4"));
  BOOST_CHECK_EQUAL("a", ParentPath(ParentPath("a/b/a")));
}

BOOST_AUTO_TEST_CASE(last_name) {
  BOOST_CHECK_EQUAL("", LastName(""));
  BOOST_CHECK_EQUAL("root", LastName("root"));
  BOOST_CHECK_EQUAL("file", LastName("root/file"));
  BOOST_CHECK_EQUAL("c", LastName("a/b/c"));
  BOOST_CHECK_EQUAL("c", LastName("a/a and b/c"));
  BOOST_CHECK_EQUAL("4", LastName("1/2/3/4"));
  BOOST_CHECK_EQUAL("a", LastName(LastName("a/b/a")));
}

BOOST_AUTO_TEST_CASE(depth) {
  BOOST_CHECK_EQUAL(0, Depth(""));
  BOOST_CHECK_EQUAL(1, Depth("a"));
  BOOST_CHECK_EQUAL(2, Depth("a/b"));
}

BOOST_AUTO_TEST_CASE(root) {
  std::string path = "";
  BOOST_CHECK_EQUAL("", Root(path));
  path = "root";
  BOOST_CHECK_EQUAL("root", Root(path));
  path = "root/file";
  BOOST_CHECK_EQUAL("root", Root(path));
  path = "a/b/c";
  BOOST_CHECK_EQUAL("a", Root(path));
  path = "a/a and b/c";
  BOOST_CHECK_EQUAL("a", Root(path));
  path = "1/2/3/4";
  BOOST_CHECK_EQUAL("1", Root(path));
  path = "a/b/a";
  BOOST_CHECK_EQUAL("a", Root(path));
}

BOOST_AUTO_TEST_CASE(remove_root) {
  std::string path = "";
  BOOST_CHECK_EQUAL("", RemoveRoot(path));
  path = "root";
  BOOST_CHECK_EQUAL("", RemoveRoot(path));
  path = "root/file";
  BOOST_CHECK_EQUAL("file", RemoveRoot(path));
  path = "a/b/c";
  BOOST_CHECK_EQUAL("b/c", RemoveRoot(path));
  path = "a/a and b/c";
  BOOST_CHECK_EQUAL("a and b/c", RemoveRoot(path));
  path = "1/2/3/4";
  BOOST_CHECK_EQUAL("2/3/4", RemoveRoot(path));
  path = "a/b/a";
  BOOST_CHECK_EQUAL("b/a", RemoveRoot(path));
}

BOOST_AUTO_TEST_CASE(short_description) {
  BOOST_CHECK_EQUAL(ShortDescription("", 0), "");
  BOOST_CHECK_EQUAL(ShortDescription("a", 0), "");
  BOOST_CHECK_EQUAL(ShortDescription("a", 1), "a");
  BOOST_CHECK_EQUAL(ShortDescription("a/b", 3), "a/b");
  BOOST_CHECK_EQUAL(ShortDescription("abc", 2), "[.");
  BOOST_CHECK_EQUAL(ShortDescription("abcde", 4), "[..]");
  BOOST_CHECK_EQUAL(ShortDescription("abcde", 5), "abcde");
  BOOST_CHECK_EQUAL(ShortDescription("abcdef", 5), "a[..]");
  BOOST_CHECK_EQUAL(ShortDescription("a/bcdefg", 5), "[..]g");
  BOOST_CHECK_EQUAL(ShortDescription("abcde/fghij", 10), "[..]/fghij");
  BOOST_CHECK_EQUAL(ShortDescription("abcdeg/f", 7), "a[..]/f");
  BOOST_CHECK_EQUAL(ShortDescription("a/bcdef/g", 8), "a/[..]/g");
  BOOST_CHECK_EQUAL(ShortDescription("a/bcdefg/h", 9), "a/[..]g/h");
}
BOOST_AUTO_TEST_SUITE_END()
