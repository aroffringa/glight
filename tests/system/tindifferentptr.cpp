#include "system/indifferentptr.h"

#include <boost/test/unit_test.hpp>

using glight::system::IndifferentPtr;
using glight::system::MakeIndifferent;

namespace {

size_t n_constructions;
size_t n_deletes;

void reset() {
  n_constructions = 0;
  n_deletes = 0;
}

class Tracker {
 public:
  Tracker() { ++n_constructions; }
  Tracker(int, int) { ++n_constructions; }
  ~Tracker() { ++n_deletes; }
};
}  // namespace

BOOST_AUTO_TEST_SUITE(indifferent_ptr)

BOOST_AUTO_TEST_CASE(empty_construct) {
  reset();
  IndifferentPtr a;
  BOOST_CHECK(!a);
  BOOST_CHECK_EQUAL(n_constructions, 0);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  a.Reset<Tracker>();
  BOOST_CHECK(!a);
  BOOST_CHECK_EQUAL(n_constructions, 0);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  BOOST_CHECK(a == IndifferentPtr());
  BOOST_CHECK(!(a != IndifferentPtr()));
}

BOOST_AUTO_TEST_CASE(pointer_construct) {
  reset();
  {
    Tracker* tracker = new Tracker();
    IndifferentPtr a(tracker);
    BOOST_CHECK(bool(a));
    BOOST_CHECK_EQUAL(&a.Get<Tracker>(), tracker);
    BOOST_CHECK_EQUAL(n_constructions, 1);
    BOOST_CHECK_EQUAL(n_deletes, 0);
    BOOST_CHECK(a != IndifferentPtr());
    BOOST_CHECK(!(a == IndifferentPtr()));
    a.Reset<Tracker>();
    BOOST_CHECK_EQUAL(n_constructions, 1);
    BOOST_CHECK_EQUAL(n_deletes, 1);
  }
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
}

BOOST_AUTO_TEST_CASE(move_construct) {
  reset();
  Tracker* tracker = new Tracker();
  IndifferentPtr a(tracker);
  IndifferentPtr b(std::move(a));
  BOOST_CHECK(!a);
  BOOST_CHECK(b);
  BOOST_CHECK_EQUAL(tracker, &b.Get<Tracker>());
  b.Reset<Tracker>();
  BOOST_CHECK(!a);
  BOOST_CHECK(!b);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
}

BOOST_AUTO_TEST_CASE(move_assignment) {
  reset();
  Tracker* tracker = new Tracker();
  IndifferentPtr a(tracker);
  IndifferentPtr b;
  b = std::move(a);
  BOOST_CHECK_EQUAL(tracker, &b.Get<Tracker>());
  BOOST_CHECK(!a);
  BOOST_CHECK(b);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);
}

BOOST_AUTO_TEST_CASE(make_indifferent_ptr) {
  reset();
  IndifferentPtr a = MakeIndifferent<Tracker>();
  BOOST_CHECK(bool(a));

  IndifferentPtr b = MakeIndifferent<Tracker>(3, 5);
  BOOST_CHECK(bool(b));
  BOOST_CHECK(a != b);
  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  a.Reset<Tracker>();
  b.Reset<Tracker>();
  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 2);
}

BOOST_AUTO_TEST_SUITE_END()
