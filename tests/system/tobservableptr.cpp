#include "system/observableptr.h"

#include <boost/test/unit_test.hpp>

using glight::system::MakeObservable;
using glight::system::ObservablePtr;
using glight::system::Observer;

namespace {

size_t n_constructions;
size_t n_deletes;

void reset() {
  n_constructions = 0;
  n_deletes = 0;
}

struct Tracker {
  Tracker() { ++n_constructions; }
  Tracker(int, int) { ++n_constructions; }
  ~Tracker() { ++n_deletes; }
  int field = 1337 + n_constructions;
};
}  // namespace

BOOST_AUTO_TEST_SUITE(observable_ptr)

BOOST_AUTO_TEST_CASE(empty_construct) {
  reset();
  ObservablePtr<Tracker> a;
  BOOST_CHECK(!a);
  BOOST_CHECK(!a.GetObserver());
  BOOST_CHECK_EQUAL(a.ShareCount(), 0);
  BOOST_CHECK_EQUAL(n_constructions, 0);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  a.Reset();
  BOOST_CHECK(!a);
  BOOST_CHECK(!a.GetObserver());
  BOOST_CHECK_EQUAL(a.ShareCount(), 0);
  BOOST_CHECK_EQUAL(n_constructions, 0);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  BOOST_CHECK(a == ObservablePtr<Tracker>());
  BOOST_CHECK(!(a != ObservablePtr<Tracker>()));
  BOOST_CHECK(a.GetObserver() == a.GetObserver());
  BOOST_CHECK_EQUAL(a.ShareCount(), 0);
}

BOOST_AUTO_TEST_CASE(pointer_construct) {
  reset();
  {
    Tracker* tracker = new Tracker();
    ObservablePtr a(tracker);
    BOOST_CHECK(bool(a));
    BOOST_CHECK_EQUAL(a.Get(), tracker);
    BOOST_CHECK_EQUAL(&*a, tracker);
    BOOST_CHECK_EQUAL(a->field, 1337);
    BOOST_CHECK_EQUAL(n_constructions, 1);
    BOOST_CHECK_EQUAL(n_deletes, 0);
    BOOST_CHECK(a != ObservablePtr<Tracker>());
    BOOST_CHECK(!(a == ObservablePtr<Tracker>()));
    BOOST_CHECK_EQUAL(a.ShareCount(), 0);

    const Observer x = a.GetObserver();
    BOOST_CHECK_EQUAL(a.ShareCount(), 1);
    const Observer y = a.GetObserver();
    BOOST_CHECK_EQUAL(a.ShareCount(), 2);
    BOOST_CHECK(static_cast<bool>(x));
    BOOST_CHECK(x == y);

    a.Reset();
    BOOST_CHECK(!x);
    BOOST_CHECK_EQUAL(a.ShareCount(), 0);
    BOOST_CHECK(x == y);
    BOOST_CHECK(x == a.GetObserver());
    BOOST_CHECK_EQUAL(n_constructions, 1);
    BOOST_CHECK_EQUAL(n_deletes, 1);
  }
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
}

BOOST_AUTO_TEST_CASE(move_construct) {
  reset();
  // Test without observers
  Tracker* tracker = new Tracker();
  ObservablePtr a(tracker);
  ObservablePtr b(std::move(a));
  BOOST_CHECK(!a);
  BOOST_CHECK(b);
  BOOST_CHECK_EQUAL(tracker, b.Get());
  b.Reset();
  BOOST_CHECK(!a);
  BOOST_CHECK(!b);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);

  // Test with observers
  tracker = new Tracker();
  ObservablePtr c(tracker);
  const Observer x = c.GetObserver();
  const Observer y = c.GetObserver();
  BOOST_CHECK_EQUAL(c.ShareCount(), 2);
  BOOST_CHECK(x == y);
  BOOST_CHECK(c.Get() == tracker);
  BOOST_CHECK(x.Get() == tracker);
  ObservablePtr d(std::move(c));
  BOOST_CHECK(!c);
  BOOST_CHECK(static_cast<bool>(d));
  BOOST_CHECK(static_cast<bool>(x));
  BOOST_CHECK_EQUAL(c.ShareCount(), 0);
  BOOST_CHECK_EQUAL(d.ShareCount(), 2);
  BOOST_CHECK(x.Get() == tracker);
  BOOST_CHECK(x == y);
  d.Reset();
  BOOST_CHECK(!x);
  BOOST_CHECK_EQUAL(d.ShareCount(), 0);
  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 2);
}

BOOST_AUTO_TEST_CASE(move_assignment) {
  reset();
  Tracker* tracker = new Tracker();
  ObservablePtr<Tracker> a(tracker);
  const Observer x = a.GetObserver();
  ObservablePtr<Tracker> b;
  BOOST_CHECK_EQUAL(b.ShareCount(), 0);
  b = std::move(a);
  BOOST_CHECK_EQUAL(tracker, b.Get());
  BOOST_CHECK(!a);
  BOOST_CHECK(b);
  BOOST_CHECK_EQUAL(b.ShareCount(), 1);
  BOOST_CHECK_EQUAL(a.ShareCount(), 0);
  BOOST_CHECK_EQUAL(x.Get(), tracker);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  b.Reset();
  BOOST_CHECK(!b);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
}

BOOST_AUTO_TEST_CASE(move_observer) {
  reset();
  Tracker* tracker = new Tracker();
  ObservablePtr<Tracker> a(tracker);
  Observer w = a.GetObserver();
  Observer x(w);
  Observer y = std::move(x);
  BOOST_CHECK(w);
  BOOST_CHECK(!x);
  BOOST_CHECK(static_cast<bool>(y));
  BOOST_CHECK_EQUAL(a.ShareCount(), 2);
  BOOST_CHECK(w.Get() == tracker);
  BOOST_CHECK(y.Get() == tracker);
  a.Reset();
  BOOST_CHECK(!w);
  BOOST_CHECK(!x);
  BOOST_CHECK(!y);
  BOOST_CHECK_EQUAL(a.ShareCount(), 0);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);

  tracker = new Tracker();
  a = ObservablePtr<Tracker>(tracker);
  w = a.GetObserver();
  x = w;
  y = a.GetObserver();
  const Observer z = a.GetObserver();
  x = std::move(y);
  BOOST_CHECK(static_cast<bool>(w));
  BOOST_CHECK(static_cast<bool>(x));
  BOOST_CHECK(!y);
  BOOST_CHECK(static_cast<bool>(z));
  a.Reset();
  BOOST_CHECK(!w);
  BOOST_CHECK(!x);
  BOOST_CHECK(!y);
  BOOST_CHECK(!z);

  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 2);
}

BOOST_AUTO_TEST_CASE(make_observable_ptr) {
  reset();
  ObservablePtr a = MakeObservable<Tracker>();
  BOOST_CHECK(bool(a));

  ObservablePtr b = MakeObservable<Tracker>(3, 5);
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
