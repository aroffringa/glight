#include "../theatre/dummydevice.h"
#include "../theatre/management.h"
#include "../theatre/presetcollection.h"
#include "../theatre/timesequence.h"

#include "../theatre/effects/fadeeffect.h"

#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_SUITE(management)

BOOST_AUTO_TEST_CASE(Destruct) {
  Management management;
  management.AddDevice(std::make_unique<DummyDevice>());
  management.Run();
  BOOST_CHECK_THROW(management.Run(), std::exception);
  management.StartBeatFinder();
}

BOOST_AUTO_TEST_CASE(HasCycles) {
  Management management;
  BOOST_CHECK_EQUAL(management.HasCycle(), false);

  std::unique_ptr<FadeEffect> effectPtr(new FadeEffect());
  Effect &effect = management.AddEffect(std::move(effectPtr));
  BOOST_CHECK_EQUAL(management.HasCycle(), false);
  effect.AddConnection(effect, 0);
  BOOST_CHECK_EQUAL(management.HasCycle(), true);
  effect.RemoveConnection(0);
  BOOST_CHECK_EQUAL(management.HasCycle(), false);

  TimeSequence &timeSeq = management.AddTimeSequence();
  BOOST_CHECK_EQUAL(management.HasCycle(), false);
  timeSeq.AddStep(timeSeq, 0);
  BOOST_CHECK_EQUAL(management.HasCycle(), true);
  timeSeq.RemoveStep(0);
  BOOST_CHECK_EQUAL(management.HasCycle(), false);

  timeSeq.AddStep(effect, 0);
  BOOST_CHECK_EQUAL(management.HasCycle(), false);
  effect.AddConnection(timeSeq, 0);
  BOOST_CHECK_EQUAL(management.HasCycle(), true);
  timeSeq.RemoveStep(0);
  BOOST_CHECK_EQUAL(management.HasCycle(), false);
  effect.RemoveConnection(0);
  BOOST_CHECK_EQUAL(management.HasCycle(), false);

  PresetCollection &collection = management.AddPresetCollection();
  BOOST_CHECK_EQUAL(management.HasCycle(), false);
  collection.AddPresetValue(collection, 0);
  BOOST_CHECK_EQUAL(management.HasCycle(), true);
  collection.Clear();
  BOOST_CHECK_EQUAL(management.HasCycle(), false);
  collection.AddPresetValue(timeSeq, 0);
  BOOST_CHECK_EQUAL(management.HasCycle(), false);
  timeSeq.AddStep(effect, 0);
  BOOST_CHECK_EQUAL(management.HasCycle(), false);
  effect.AddConnection(collection, 0);
  BOOST_CHECK_EQUAL(management.HasCycle(), true);
}

BOOST_AUTO_TEST_SUITE_END()
