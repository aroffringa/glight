#include "system/trackableptr.h"

#include <optional>

#include <boost/test/unit_test.hpp>

using glight::system::MakeTrackable;
using glight::system::ObservingPtr;
using glight::system::TrackablePtr;

namespace {

size_t n_constructions;
size_t n_deletes;
constexpr size_t kFieldStart = 1337;

void ResetTracker() {
  n_constructions = 0;
  n_deletes = 0;
}

struct Tracker {
  Tracker() { ++n_constructions; }
  Tracker(int, int) { ++n_constructions; }
  ~Tracker() { ++n_deletes; }
  int field = kFieldStart + n_constructions;
};
}  // namespace

namespace glight::system {
template class TrackablePtr<Tracker>;
}

BOOST_AUTO_TEST_SUITE(trackable_ptr)

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

BOOST_AUTO_TEST_CASE(self_assignment_trackable_ptr) {
  ResetTracker();
  TrackablePtr<Tracker> empty;
  TrackablePtr<Tracker>* copy = &empty;
  *copy = std::move(empty);
  BOOST_CHECK(!empty);

  Tracker* tracker = new Tracker();
  TrackablePtr<Tracker> a(tracker);
  copy = &a;
  *copy = std::move(a);
  BOOST_CHECK_EQUAL(a.Get(), tracker);

  ObservingPtr<Tracker> x = a.GetObserver();
  ObservingPtr<Tracker> y = x;
  ObservingPtr<Tracker> z = a.GetObserver();
  *copy = std::move(a);
  BOOST_CHECK_EQUAL(a.Get(), tracker);
  BOOST_CHECK_EQUAL(x.Get(), tracker);
  BOOST_CHECK_EQUAL(y.Get(), tracker);
  BOOST_CHECK_EQUAL(z.Get(), tracker);
  BOOST_CHECK_EQUAL(a.ShareCount(), 3);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  a.Reset();
  BOOST_CHECK(!a && !x && !y && !z);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
}

BOOST_AUTO_TEST_CASE(self_copy_assignment_observing_ptr) {
  ResetTracker();
  ObservingPtr<Tracker> empty;
  ObservingPtr<Tracker>& empty_alias = empty;
  empty_alias = empty;
  BOOST_CHECK(!empty_alias);

  Tracker* tracker = new Tracker();
  TrackablePtr<Tracker> a(tracker);

  ObservingPtr<Tracker> x = a.GetObserver();
  ObservingPtr<Tracker> y = x;
  ObservingPtr<Tracker> z = a.GetObserver();
  ObservingPtr<Tracker>& x_alias = x;
  x_alias = x;
  BOOST_CHECK_EQUAL(x.Get(), tracker);
  BOOST_CHECK_EQUAL(a.ShareCount(), 3);
  BOOST_CHECK_EQUAL(a.Get(), tracker);
  BOOST_CHECK_EQUAL(y.Get(), tracker);
  BOOST_CHECK_EQUAL(z.Get(), tracker);
  ObservingPtr<Tracker>& y_alias = y;
  y_alias = y;
  ObservingPtr<Tracker>& z_alias = z;
  z_alias = z;
  BOOST_CHECK_EQUAL(a.ShareCount(), 3);
  BOOST_CHECK_EQUAL(a.Get(), tracker);
  BOOST_CHECK_EQUAL(x.Get(), tracker);
  BOOST_CHECK_EQUAL(y.Get(), tracker);
  BOOST_CHECK_EQUAL(z.Get(), tracker);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  a.Reset();
  BOOST_CHECK(!a && !x && !y && !z);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
}

BOOST_AUTO_TEST_CASE(self_move_assignment_observing_ptr) {
  ResetTracker();
  ObservingPtr<Tracker> empty;
  ObservingPtr<Tracker>& empty_alias = empty;
  empty_alias = std::move(empty);
  BOOST_CHECK(!empty_alias);

  Tracker* tracker = new Tracker();
  TrackablePtr<Tracker> a(tracker);

  ObservingPtr<Tracker> x = a.GetObserver();
  ObservingPtr<Tracker> y = x;
  ObservingPtr<Tracker> z = a.GetObserver();
  ObservingPtr<Tracker>& x_alias = x;
  x_alias = std::move(x);
  BOOST_CHECK_EQUAL(x.Get(), tracker);
  BOOST_CHECK_EQUAL(a.ShareCount(), 3);
  BOOST_CHECK_EQUAL(a.Get(), tracker);
  BOOST_CHECK_EQUAL(y.Get(), tracker);
  BOOST_CHECK_EQUAL(z.Get(), tracker);
  ObservingPtr<Tracker>& y_alias = y;
  y_alias = std::move(y);
  ObservingPtr<Tracker>& z_alias = z;
  z_alias = std::move(z);
  BOOST_CHECK_EQUAL(a.ShareCount(), 3);
  BOOST_CHECK_EQUAL(a.Get(), tracker);
  BOOST_CHECK_EQUAL(x.Get(), tracker);
  BOOST_CHECK_EQUAL(y.Get(), tracker);
  BOOST_CHECK_EQUAL(z.Get(), tracker);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  a.Reset();
  BOOST_CHECK(!a && !x && !y && !z);
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

BOOST_AUTO_TEST_CASE(self_swap_trackable_ptr) {
  ResetTracker();
  TrackablePtr<Tracker> empty;
  swap(empty, empty);
  BOOST_CHECK(!empty);
  BOOST_CHECK_EQUAL(empty.ShareCount(), 0);

  Tracker* tracker_a = new Tracker();
  TrackablePtr<Tracker> a(tracker_a);
  swap(a, a);
  BOOST_CHECK_EQUAL(a.ShareCount(), 0);
  BOOST_CHECK_EQUAL(a->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(a.Get(), tracker_a);

  ObservingPtr<Tracker> x = a.GetObserver();
  swap(a, a);
  BOOST_CHECK_EQUAL(a.ShareCount(), 1);
  BOOST_CHECK_EQUAL(a->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(x->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(a.Get(), tracker_a);
  BOOST_CHECK_EQUAL(x.Get(), tracker_a);

  ObservingPtr<Tracker> y = a.GetObserver();
  ObservingPtr<Tracker> z = y;
  swap(a, a);
  BOOST_CHECK_EQUAL(a.ShareCount(), 3);
  BOOST_CHECK_EQUAL(a->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(x->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(y->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(z->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(a.Get(), tracker_a);
  BOOST_CHECK_EQUAL(x.Get(), tracker_a);
  BOOST_CHECK_EQUAL(y.Get(), tracker_a);
  BOOST_CHECK_EQUAL(z.Get(), tracker_a);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 0);
  a.Reset();
  BOOST_CHECK(!a);
  BOOST_CHECK(!x);
  BOOST_CHECK(!y);
  BOOST_CHECK(!z);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
}

BOOST_AUTO_TEST_CASE(self_swap_observing_ptr) {
  ResetTracker();
  ObservingPtr<Tracker> empty;
  swap(empty, empty);
  BOOST_CHECK(!empty);

  Tracker* tracker_a = new Tracker();
  TrackablePtr<Tracker> a(tracker_a);
  ObservingPtr<Tracker> x = a.GetObserver();
  swap(x, x);
  BOOST_CHECK_EQUAL(a.ShareCount(), 1);
  BOOST_CHECK_EQUAL(a->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(x->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(a.Get(), tracker_a);
  BOOST_CHECK_EQUAL(x.Get(), tracker_a);

  ObservingPtr<Tracker> y = x;
  ObservingPtr<Tracker> z = a.GetObserver();
  swap(y, y);
  swap(z, z);
  BOOST_CHECK_EQUAL(a.ShareCount(), 3);
  BOOST_CHECK_EQUAL(a->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(x->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(y->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(z->field - kFieldStart, 0);
  BOOST_CHECK_EQUAL(a.Get(), tracker_a);
  BOOST_CHECK_EQUAL(x.Get(), tracker_a);
  BOOST_CHECK_EQUAL(y.Get(), tracker_a);
  BOOST_CHECK_EQUAL(z.Get(), tracker_a);

  a.Reset();
  BOOST_CHECK(!a);
  BOOST_CHECK(!x);
  BOOST_CHECK(!y);
  BOOST_CHECK(!z);
  BOOST_CHECK_EQUAL(n_constructions, 1);
  BOOST_CHECK_EQUAL(n_deletes, 1);
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

BOOST_AUTO_TEST_CASE(in_container) {
  ResetTracker();
  std::vector<TrackablePtr<Tracker>> owners;
  std::vector<ObservingPtr<Tracker>> observers;
  for (size_t i = 0; i != 9; ++i) {
    const TrackablePtr<Tracker>& tracker =
        owners.emplace_back(MakeTrackable<Tracker>());
    observers.emplace_back(tracker.GetObserver());
    observers.emplace_back(tracker.GetObserver());
    BOOST_CHECK_EQUAL(owners[i].ShareCount(), 2);
  }
  observers.erase(observers.begin() + 1);
  for (size_t i = 0; i != 9; ++i) {
    BOOST_CHECK_EQUAL(owners[i].ShareCount(), i == 0 ? 1 : 2);
  }
  observers.erase(observers.begin() + 1);
  for (size_t i = 0; i != 9; ++i) {
    BOOST_CHECK_EQUAL(owners[i].ShareCount(), i < 2 ? 1 : 2);
  }

  observers.erase(observers.begin() + 5);
  observers.erase(observers.begin() + 2);
  observers.erase(observers.begin() + 5);
  observers.erase(observers.begin() + 5);
  observers.erase(observers.begin() + 9);
  observers.erase(observers.begin() + 6);
  observers.erase(observers.begin() + 9);
  for (size_t i = 0; i != 9; ++i) {
    BOOST_REQUIRE(owners[i]);
    BOOST_CHECK_EQUAL(owners[i]->field - kFieldStart, i);
    BOOST_CHECK_EQUAL(owners[i].ShareCount(), 1);
    BOOST_REQUIRE(observers[i]);
    BOOST_CHECK_EQUAL(observers[i]->field - kFieldStart, i);
  }
  owners.erase(owners.begin() + 4);
  owners.erase(owners.begin() + 0);
  owners.erase(owners.begin() + 6);
  owners.erase(owners.begin() + 1);
  owners.erase(owners.begin() + 3);
  BOOST_CHECK_EQUAL(owners[0]->field, kFieldStart + 1);
  BOOST_CHECK_EQUAL(owners[1]->field, kFieldStart + 3);
  BOOST_CHECK_EQUAL(owners[2]->field, kFieldStart + 5);
  BOOST_CHECK_EQUAL(owners[3]->field, kFieldStart + 7);
  for (size_t i = 0; i != 10; i += 2) BOOST_CHECK(!observers[i]);
  for (size_t i = 1; i != 9; i += 2) BOOST_CHECK(observers[i]);
}

BOOST_AUTO_TEST_CASE(make_trackable_ptr) {
  ResetTracker();
  TrackablePtr a = MakeTrackable<Tracker>();
  BOOST_CHECK(bool(a));

  TrackablePtr b = MakeTrackable<Tracker>(3, 5);
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
