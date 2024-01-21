#include "theatre/management.h"

#include "theatre/scenes/scene.h"

#include <boost/test/unit_test.hpp>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(scene)

BOOST_AUTO_TEST_CASE(make) {
  Management management;
  Scene& scene = management.AddScene(true);
  BOOST_CHECK_EQUAL(management.Controllables().size(), 1);
  KeySceneItem& item = *scene.AddKeySceneItem(1250);
  BOOST_CHECK_EQUAL(scene.SceneItems().begin()->first, 1250);
  BOOST_CHECK(scene.SceneItems().begin()->second.get() == &item);
  item.SetDurationInMS(750);
  BOOST_CHECK_EQUAL(item.OffsetInMS(), 1250);
  BOOST_CHECK_EQUAL(item.DurationInMS(), 750);
  scene.ChangeSceneItemStartTime(&item, 2000);
  BOOST_CHECK_EQUAL(scene.SceneItems().begin()->first, 2000);
  BOOST_CHECK_EQUAL(item.OffsetInMS(), 2000);
  scene.Remove(&item);
  BOOST_CHECK(scene.SceneItems().empty());
}

BOOST_AUTO_TEST_SUITE_END()
