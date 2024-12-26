#include "system/deletableptr.h"

#include <boost/test/unit_test.hpp>

using glight::system::DeletablePtr;
using glight::system::MakeDeletable;

namespace {

size_t n_constructions;
size_t n_deletes;

void ResetTracker() {
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

BOOST_AUTO_TEST_SUITE(deletable_ptr)

BOOST_AUTO_TEST_CASE(empty_construct) {
  ResetTracker();
  DeletablePtr<Tracker> a;
  BOOST_CHECK(!a);
  BOOST_CHECK(a.Get() == nullptr);
  BOOST_CHECK_EQUAL(n_constructions, 0);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  a.Reset();
  BOOST_CHECK(!a);
  BOOST_CHECK(a.Get() == nullptr);
  BOOST_CHECK_EQUAL(n_constructions, 0);
  BOOST_CHECK_EQUAL(n_deletes, 0);
}

BOOST_AUTO_TEST_CASE(value_construct) {
  ResetTracker();
  {
    Tracker* t = new Tracker();
    DeletablePtr<Tracker> b(t);
    BOOST_CHECK(bool(b));
    BOOST_CHECK_EQUAL(b.Get(), t);
    BOOST_CHECK_EQUAL(n_constructions, 1);
    BOOST_CHECK_EQUAL(n_deletes, 0);
  }
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
  {
    Tracker* t = new Tracker();
    DeletablePtr<Tracker> b =
        DeletablePtr<Tracker>(std::unique_ptr<Tracker>(t));
    BOOST_CHECK(bool(b));
    BOOST_CHECK_EQUAL(b.Get(), t);
    BOOST_CHECK_EQUAL(n_constructions, 2);
    BOOST_CHECK_EQUAL(n_deletes, 1);
  }
  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 2);
}

BOOST_AUTO_TEST_CASE(delete_method) {
  ResetTracker();
  {
    DeletablePtr<Tracker> c(new Tracker());
    c.Delete();
    BOOST_CHECK(!c);
    BOOST_CHECK(c.Get() == nullptr);
    BOOST_CHECK_EQUAL(n_constructions, 1);
    BOOST_CHECK_EQUAL(n_deletes, 1);
  }
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
}

BOOST_AUTO_TEST_CASE(copy_construct) {
  ResetTracker();
  DeletablePtr<Tracker> a(new Tracker());
  DeletablePtr<Tracker> e(a);
  BOOST_CHECK_EQUAL(a.Get(), e.Get());
  e.Delete();
  BOOST_CHECK(!a);
  BOOST_CHECK(!e);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
}

BOOST_AUTO_TEST_CASE(move_construct) {
  ResetTracker();
  Tracker* tracker = new Tracker();
  DeletablePtr<Tracker> a(tracker);
  DeletablePtr<Tracker> e(std::move(a));
  BOOST_CHECK(!a);
  BOOST_CHECK(e.Get() == tracker);
  e.Delete();
  BOOST_CHECK(!a);
  BOOST_CHECK(!e);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
}

BOOST_AUTO_TEST_CASE(copy_assignment) {
  ResetTracker();
  Tracker* tracker_d = new Tracker();
  DeletablePtr<Tracker> a;
  DeletablePtr<Tracker> d(tracker_d);
  a = d;
  BOOST_CHECK(bool(a));
  BOOST_CHECK(bool(d));
  BOOST_CHECK_EQUAL(a.Get(), tracker_d);
  BOOST_CHECK_EQUAL(a.Get(), d.Get());
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);
}

BOOST_AUTO_TEST_CASE(move_assignment) {
  ResetTracker();
  DeletablePtr<Tracker> a;
  DeletablePtr<Tracker> f(new Tracker());
  a = std::move(f);
  BOOST_CHECK(bool(a));
  BOOST_CHECK(!f);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);
}

BOOST_AUTO_TEST_CASE(make_deletable_ptr) {
  ResetTracker();
  DeletablePtr<Tracker> a = MakeDeletable<Tracker>();
  BOOST_CHECK(bool(a));

  DeletablePtr<Tracker> b = MakeDeletable<Tracker>(3, 5);
  BOOST_CHECK(bool(b));
  BOOST_CHECK(a != b);
  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  a.Reset();
  b.Reset();
  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 2);
}

BOOST_AUTO_TEST_SUITE_END()
