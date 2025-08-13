#include "system/settings.h"

#include "theatre/chase.h"
#include "theatre/fixturecontrol.h"
#include "theatre/fixturetype.h"
#include "theatre/folder.h"
#include "theatre/folderoperations.h"
#include "theatre/management.h"
#include "theatre/theatre.h"
#include "theatre/timesequence.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;
using glight::system::ObservingPtr;
using glight::system::TrackablePtr;

BOOST_AUTO_TEST_SUITE(folder)

BOOST_AUTO_TEST_CASE(AddItem) {
  TrackablePtr<Folder> a(new Folder("a"));
  BOOST_CHECK_EQUAL(a->Children().size(), 0);
  TrackablePtr<Folder> b(new Folder("b"));
  BOOST_CHECK_EQUAL(b->Children().size(), 0);
  a->Add(b.GetObserver());
  BOOST_CHECK_EQUAL(a->Children().size(), 1);
  BOOST_CHECK_EQUAL(b->Children().size(), 0);
  BOOST_CHECK_EQUAL(b->Parent().Name(), a->Name());

  // Check if duplicate names in one folder generate an exception
  TrackablePtr<Folder> b2(new Folder("b"));
  BOOST_CHECK_THROW(a->Add(b2.GetObserver()), std::exception);
}

BOOST_AUTO_TEST_CASE(Move) {
  TrackablePtr<Folder> a(new Folder("a"));
  TrackablePtr<Folder> b(new Folder("b"));
  TrackablePtr<Folder> c(new Folder("c"));
  a->Add(b.GetObserver());
  Folder::Move(b.GetObserver(), *c);
  BOOST_CHECK_EQUAL(a->Children().size(), 0);
  BOOST_CHECK_EQUAL(b->Children().size(), 0);
  BOOST_CHECK_EQUAL(c->Children().size(), 1);
  BOOST_CHECK_EQUAL(c->GetChild("b").Name(), "b");
  Folder::Move(b.GetObserver(), *a);
  BOOST_CHECK_EQUAL(a->Children().size(), 1);
  BOOST_CHECK_EQUAL(a->GetChild("b").Name(), "b");
  Folder::Move(b.GetObserver(), *a);
}

BOOST_AUTO_TEST_CASE(FullPath) {
  TrackablePtr<Folder> a(new Folder("a"));
  BOOST_CHECK_EQUAL(a->FullPath(), "a");
  TrackablePtr<Folder> b(new Folder("b"));
  BOOST_CHECK_EQUAL(b->FullPath(), "b");
  a->Add(b.GetObserver());
  BOOST_CHECK_EQUAL(b->FullPath(), "a/b");
  TrackablePtr<Folder> c(new Folder("c"));
  b->Add(c.GetObserver());
  BOOST_CHECK_EQUAL(a->FullPath(), "a");
  BOOST_CHECK_EQUAL(b->FullPath(), "a/b");
  BOOST_CHECK_EQUAL(c->FullPath(), "a/b/c");
}

BOOST_AUTO_TEST_CASE(FollowDown) {
  TrackablePtr<Folder> a(new Folder("a"));
  TrackablePtr<Folder> b(new Folder("b"));
  TrackablePtr<Folder> c(new Folder("c"));
  b->Add(c.GetObserver());
  a->Add(b.GetObserver());

  BOOST_CHECK_EQUAL(a->FollowDown("b"), b.Get());

  BOOST_CHECK_EQUAL(a->FollowDown("b/c"), c.Get());

  BOOST_CHECK_EQUAL(b->FollowDown("c"), c.Get());

  TrackablePtr<Folder> root(new Folder("root"));
  TrackablePtr<Folder> f1(new Folder("bert"));
  TrackablePtr<Folder> f2(new Folder("carole"));
  TrackablePtr<Folder> f3(new Folder("daniel"));
  f2->Add(f3.GetObserver());
  f1->Add(f2.GetObserver());
  root->Add(f1.GetObserver());
  BOOST_CHECK_EQUAL(
      root->FollowDown(folders::RemoveRoot("root/bert/carole/daniel")),
      f3.Get());
  std::string notMoved = folders::RemoveRoot("root/bert/carole/daniel");
  BOOST_CHECK_EQUAL(root->FollowDown(notMoved), f3.Get());
}

BOOST_AUTO_TEST_CASE(FollowRelPath) {
  TrackablePtr<Folder> a(new Folder("a"));
  TrackablePtr<Folder> b(new Folder("b"));
  TrackablePtr<FolderObject> c(new FolderObject("c"));

  b->Add(c.GetObserver());
  a->Add(b.GetObserver());

  BOOST_CHECK_EQUAL(a->FollowRelPath("b"), b.Get());

  BOOST_CHECK_EQUAL(a->FollowRelPath("b/c"), c.Get());

  BOOST_CHECK_EQUAL(b->FollowRelPath("c"), c.Get());
}

BOOST_AUTO_TEST_CASE(GetAvailableName) {
  TrackablePtr<Folder> a(new Folder("a"));
  BOOST_CHECK_EQUAL(a->GetAvailableName("obj"), "obj1");
  BOOST_CHECK_EQUAL(a->GetAvailableName(""), "1");

  TrackablePtr<Folder> b(new Folder("obj1"));
  a->Add(b.GetObserver());
  BOOST_CHECK_EQUAL(a->GetAvailableName("obj"), "obj2");
  BOOST_CHECK_EQUAL(a->GetAvailableName(""), "1");

  TrackablePtr<Folder> c(new Folder("obj3"));
  a->Add(c.GetObserver());
  BOOST_CHECK_EQUAL(a->GetAvailableName("obj"), "obj2");

  TrackablePtr<Folder> d(new Folder("obj2"));
  a->Add(d.GetObserver());
  BOOST_CHECK_EQUAL(a->GetAvailableName("obj"), "obj4");
}

BOOST_AUTO_TEST_CASE(FolderManagement) {
  const glight::system::Settings settings;
  Management management(settings);
  Folder &root = management.RootFolder();
  BOOST_CHECK(root.Children().empty());
  root.SetName("a");
  BOOST_CHECK_EQUAL(&management.GetObjectFromPath("a"), &root);
  Folder &folderB = management.AddFolder(root, "b");
  BOOST_CHECK_EQUAL(&management.GetObjectFromPath("a/b"), &folderB);
}

BOOST_AUTO_TEST_CASE(RemoveFolder) {
  // Test removal of a folder that contains dependencies to
  // other dependent items

  const glight::system::Settings settings;
  Management management(settings);
  Folder &root = management.RootFolder();
  FixtureType &fixtureType =
      *management.GetTheatre().AddFixtureType(StockFixture::Light1Ch);
  Fixture &fixture =
      *management.GetTheatre().AddFixture(fixtureType.Modes().front());
  ObservingPtr<FixtureControl> control =
      management.AddFixtureControlPtr(fixture, root);

  Folder &folder = management.AddFolder(root, "Folder");
  ObservingPtr<TimeSequence> ts1 = management.AddTimeSequencePtr();
  ts1->SetName("ts1");
  folder.Add(ts1);

  ObservingPtr<Chase> c = management.AddChasePtr();
  c->SetName("c");
  folder.Add(c);
  c->GetSequence().Add(*control, 0);

  ts1->AddStep(*c, 0);

  ObservingPtr<TimeSequence> ts2 = management.AddTimeSequencePtr();
  ts2->SetName("ts2");
  folder.Add(ts2);
  ts2->AddStep(*c, 0);

  management.RemoveFolder(folder);
  BOOST_CHECK_EQUAL(management.Folders().size(), 1);
}

BOOST_AUTO_TEST_SUITE_END()
