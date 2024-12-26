#include "system/trackableptr.h"

#include <optional>

#include <boost/test/unit_test.hpp>

using glight::system::MakeObservable;
using glight::system::ObservingPtr;
using glight::system::TrackablePtr;

namespace {

size_t n_constructions;
size_t n_deletes;

void ResetTracker() {
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

namespace glight::system {
template class TrackablePtr<Tracker>;
}

BOOST_AUTO_TEST_SUITE(observable_ptr)

BOOST_AUTO_TEST_CASE(empty_construct) {
  ResetTracker();
  TrackablePtr<Tracker> a;
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
  BOOST_CHECK(a == TrackablePtr<Tracker>());
  BOOST_CHECK(!(a != TrackablePtr<Tracker>()));
  BOOST_CHECK(a.GetObserver() == a.GetObserver());
  BOOST_CHECK_EQUAL(a.ShareCount(), 0);

  TrackablePtr<Tracker> b(nullptr);
  BOOST_CHECK(!b);
  b.Reset(nullptr);
  BOOST_CHECK(!b);
}

BOOST_AUTO_TEST_CASE(pointer_construct) {
  ResetTracker();
  {
    Tracker* tracker = new Tracker();
    TrackablePtr a(tracker);
    BOOST_CHECK(bool(a));
    BOOST_CHECK_EQUAL(a.Get(), tracker);
    BOOST_CHECK_EQUAL(&*a, tracker);
    BOOST_CHECK_EQUAL(a->field, 1337);
    BOOST_CHECK_EQUAL(n_constructions, 1);
    BOOST_CHECK_EQUAL(n_deletes, 0);
    BOOST_CHECK(a != TrackablePtr<Tracker>());
    BOOST_CHECK(!(a == TrackablePtr<Tracker>()));
    BOOST_CHECK_EQUAL(a.ShareCount(), 0);

    const ObservingPtr x = a.GetObserver();
    BOOST_CHECK_EQUAL(a.ShareCount(), 1);
    const ObservingPtr y = a.GetObserver();
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
  {
    Tracker* tracker = new Tracker();
    std::unique_ptr<Tracker> u_tracker = std::unique_ptr<Tracker>(tracker);
    TrackablePtr p(std::move(u_tracker));
    BOOST_CHECK_EQUAL(p.Get(), tracker);
    BOOST_CHECK_EQUAL(&*p, tracker);
    BOOST_CHECK_EQUAL(p->field, 1338);
    BOOST_CHECK_EQUAL(n_constructions, 2);
    BOOST_CHECK_EQUAL(n_deletes, 1);
  }
  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 2);
}

BOOST_AUTO_TEST_CASE(reset) {
  ResetTracker();
  Tracker* tracker_a = new Tracker();
  TrackablePtr p(tracker_a);
  ObservingPtr x = p.GetObserver();
  p.Reset();
  BOOST_CHECK(!p);
  BOOST_CHECK(!x);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
  x = p.GetObserver();
  BOOST_CHECK(!p);
  p.Reset();
  BOOST_CHECK(!p);

  Tracker* tracker_b = new Tracker();
  p.Reset(tracker_b);
  BOOST_CHECK(static_cast<bool>(p));
  BOOST_CHECK(p.Get() == tracker_b);
  BOOST_CHECK(!x);
  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 1);
  x = p.GetObserver();
  BOOST_CHECK(x);

  Tracker* tracker_c = new Tracker();
  p.Reset(tracker_c);
  BOOST_CHECK(static_cast<bool>(p));
  BOOST_CHECK(p.Get() == tracker_c);
  BOOST_CHECK(!x);
  x = p.GetObserver();
  BOOST_CHECK(static_cast<bool>(x));
  BOOST_CHECK_EQUAL(n_constructions, 3);
  BOOST_CHECK_EQUAL(n_deletes, 2);

  p.Reset(std::unique_ptr<Tracker>());
  BOOST_CHECK_EQUAL(n_constructions, 3);
  BOOST_CHECK_EQUAL(n_deletes, 3);
  BOOST_CHECK(!x);
}

BOOST_AUTO_TEST_CASE(release) {
  ResetTracker();
  std::unique_ptr u = TrackablePtr<Tracker>().Release();
  BOOST_CHECK(!u);

  Tracker* tracker = new Tracker();
  TrackablePtr<Tracker> p(tracker);
  u = p.Release();
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  BOOST_CHECK(!p);
  BOOST_CHECK(u.get() == tracker);
}

BOOST_AUTO_TEST_CASE(move_construct) {
  ResetTracker();
  // Test without observers
  Tracker* tracker = new Tracker();
  TrackablePtr a(tracker);
  TrackablePtr b(std::move(a));
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
  TrackablePtr c(tracker);
  const ObservingPtr x = c.GetObserver();
  const ObservingPtr y = c.GetObserver();
  BOOST_CHECK_EQUAL(c.ShareCount(), 2);
  BOOST_CHECK(x == y);
  BOOST_CHECK(c.Get() == tracker);
  BOOST_CHECK(x.Get() == tracker);
  TrackablePtr d(std::move(c));
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
  ResetTracker();
  Tracker* tracker = new Tracker();
  TrackablePtr<Tracker> a(tracker);
  const ObservingPtr x = a.GetObserver();
  TrackablePtr<Tracker> b;
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

BOOST_AUTO_TEST_CASE(get_observer) {
  ResetTracker();
  Tracker* tracker = new Tracker();
  TrackablePtr<Tracker> a(tracker);
  std::optional<ObservingPtr<Tracker>> x = a.GetObserver();
  ObservingPtr<Tracker> y = a.GetObserver();
  ObservingPtr<Tracker> z = a.GetObserver();
  BOOST_CHECK(*x == y);
  BOOST_CHECK(y == z);
  BOOST_CHECK(x->Get() == tracker);
  BOOST_CHECK(y.Get() == tracker);
  BOOST_CHECK(z.Get() == tracker);
  x.reset();

  BOOST_CHECK(!TrackablePtr<Tracker>().GetObserver());

  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);
}

BOOST_AUTO_TEST_CASE(swap_functions) {
  ResetTracker();
  Tracker* tracker_a = new Tracker();
  TrackablePtr<Tracker> a(tracker_a);
  ObservingPtr<Tracker> x = a.GetObserver();
  ObservingPtr<Tracker> y = a.GetObserver();
  swap(x, y);
  BOOST_CHECK(x == y);
  BOOST_CHECK(x.Get() == tracker_a);
  BOOST_CHECK(y.Get() == tracker_a);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);

  Tracker* tracker_b = new Tracker();
  TrackablePtr<Tracker> b(tracker_b);
  ObservingPtr<Tracker> z = b.GetObserver();
  BOOST_CHECK(x != z);
  swap(x, z);
  BOOST_CHECK(x.Get() == tracker_b);
  BOOST_CHECK(y.Get() == tracker_a);
  BOOST_CHECK(z.Get() == tracker_a);
  BOOST_CHECK(x != z);
  BOOST_CHECK(x != y);
  BOOST_CHECK(z == y);
  swap(x, z);
  BOOST_CHECK(x.Get() == tracker_a);
  BOOST_CHECK(y.Get() == tracker_a);
  BOOST_CHECK(z.Get() == tracker_b);
  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 0);

  swap(a, b);
  BOOST_CHECK(a.Get() == tracker_b);
  BOOST_CHECK(b.Get() == tracker_a);
  BOOST_CHECK(x.Get() == tracker_a);
  BOOST_CHECK(y.Get() == tracker_a);
  BOOST_CHECK(z.Get() == tracker_b);
}

BOOST_AUTO_TEST_CASE(swap_different_ptr_observers) {
  Tracker* tracker_a = new Tracker();
  TrackablePtr<Tracker> a(tracker_a);
  ObservingPtr<Tracker> w = a.GetObserver();
  ObservingPtr<Tracker> x = a.GetObserver();

  Tracker* tracker_b = new Tracker();
  TrackablePtr<Tracker> b(tracker_b);
  ObservingPtr<Tracker> y = b.GetObserver();
  ObservingPtr<Tracker> z = b.GetObserver();

  BOOST_CHECK(w.Get() != y.Get());
  swap(w, y);
  BOOST_CHECK(w.Get() == tracker_b);
  BOOST_CHECK(y.Get() == tracker_a);
  swap(w, y);
  BOOST_CHECK(w.Get() == tracker_a);
  BOOST_CHECK(y.Get() == tracker_b);

  swap(x, z);
  BOOST_CHECK(x.Get() == tracker_b);
  BOOST_CHECK(z.Get() == tracker_a);
  swap(x, z);
  BOOST_CHECK(x.Get() == tracker_a);
  BOOST_CHECK(z.Get() == tracker_b);
  b.Reset();
}

BOOST_AUTO_TEST_CASE(move_observer) {
  ResetTracker();
  Tracker* tracker = new Tracker();
  TrackablePtr<Tracker> a(tracker);
  ObservingPtr w = a.GetObserver();
  ObservingPtr x(w);
  ObservingPtr y = std::move(x);
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
  a = TrackablePtr<Tracker>(tracker);
  w = a.GetObserver();
  x = w;
  y = a.GetObserver();
  const ObservingPtr z = a.GetObserver();
  x = std::move(y);
  BOOST_CHECK(static_cast<bool>(w));
  BOOST_CHECK(static_cast<bool>(x));
  BOOST_CHECK(!y);
  BOOST_CHECK(static_cast<bool>(z));
  y = std::move(w);
  BOOST_CHECK(!w);
  BOOST_CHECK(static_cast<bool>(x));
  BOOST_CHECK(static_cast<bool>(y));
  BOOST_CHECK(static_cast<bool>(z));
  a.Reset();
  BOOST_CHECK(!w);
  BOOST_CHECK(!x);
  BOOST_CHECK(!y);
  BOOST_CHECK(!z);

  BOOST_CHECK_EQUAL(n_constructions, 2);
  BOOST_CHECK_EQUAL(n_deletes, 2);
}

BOOST_AUTO_TEST_CASE(move_observer_after_ptr_destroyed) {
  ResetTracker();
  Tracker* tracker = new Tracker();
  std::optional<TrackablePtr<Tracker>> a(tracker);
  ObservingPtr<Tracker> x = a->GetObserver();
  a.reset();
  BOOST_CHECK(!a);
  ObservingPtr<Tracker> y(nullptr);
  y = std::move(x);
  BOOST_CHECK(!y);
}

BOOST_AUTO_TEST_CASE(make_observable_ptr) {
  ResetTracker();
  TrackablePtr a = MakeObservable<Tracker>();
  BOOST_CHECK(bool(a));

  TrackablePtr b = MakeObservable<Tracker>(3, 5);
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
