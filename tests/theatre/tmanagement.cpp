#include "theatre/chase.h"
#include "theatre/dummydevice.h"
#include "theatre/fixturecontrol.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"
#include "theatre/sourcevalue.h"
#include "theatre/theatre.h"
#include "theatre/timesequence.h"

#include "theatre/effects/fadeeffect.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;

BOOST_AUTO_TEST_SUITE(management)

BOOST_AUTO_TEST_CASE(Destruct) {
  Management management;
  management.AddDevice(std::make_unique<DummyDevice>());
  management.Run();
  BOOST_CHECK_THROW(management.Run(), std::exception);
  management.StartBeatFinder();
}

BOOST_AUTO_TEST_CASE(RemoveObject) {
  Management management;
  std::unique_ptr<FadeEffect> effectPtr(new FadeEffect());
  effectPtr->SetName("effect");
  BOOST_CHECK(management.RootFolder().Children().empty());
  Folder &folder = management.AddFolder(management.RootFolder(), "folder");
  Effect &effect = management.AddEffect(std::move(effectPtr), folder);
  management.AddSourceValue(effect, 0);
  Chase &chase = management.AddChase();
  chase.SetName("chase");
  folder.Add(chase);
  management.AddSourceValue(chase, 0);
  BOOST_CHECK_EQUAL(management.RootFolder().Children().size(), 1);
  BOOST_CHECK_EQUAL(folder.Children().size(), 2);
  BOOST_CHECK(!management.SourceValues().empty());

  management.RemoveObject(folder);

  BOOST_CHECK(management.RootFolder().Children().empty());
  BOOST_CHECK(management.SourceValues().empty());
}

BOOST_AUTO_TEST_CASE(GetSpecificControllables) {
  Management management;
  BOOST_CHECK_EQUAL(management.GetSpecificControllables<Controllable>().size(),
                    0);
  management.AddEffect(Effect::Make(EffectType::Fade));
  BOOST_CHECK_EQUAL(management.GetSpecificControllables<Controllable>().size(),
                    1);
  BOOST_CHECK_EQUAL(management.GetSpecificControllables<Effect>().size(), 1);
  BOOST_CHECK_EQUAL(management.GetSpecificControllables<FadeEffect>().size(),
                    1);
  BOOST_CHECK_EQUAL(management.GetSpecificControllables<Chase>().size(), 0);
  management.AddEffect(Effect::Make(EffectType::Variable));
  BOOST_CHECK_EQUAL(management.GetSpecificControllables<Controllable>().size(),
                    2);
  BOOST_CHECK_EQUAL(management.GetSpecificControllables<Effect>().size(), 2);
  BOOST_CHECK_EQUAL(management.GetSpecificControllables<FadeEffect>().size(),
                    1);
  BOOST_CHECK_EQUAL(management.GetSpecificControllables<Chase>().size(), 0);
}

BOOST_AUTO_TEST_CASE(RemoveUnusedFixtureType) {
  Management management;
  FixtureType &typeA =
      management.GetTheatre().AddFixtureType(StockFixture::Light1Ch);
  management.RootFolder().Add(typeA);
  FixtureType &typeB =
      management.GetTheatre().AddFixtureType(StockFixture::Rgb3Ch);
  management.RootFolder().Add(typeB);
  FixtureType &typeC =
      management.GetTheatre().AddFixtureType(StockFixture::Rgba4Ch);
  management.RootFolder().Add(typeC);
  management.RemoveFixtureType(typeB);
  management.RemoveFixtureType(typeC);
  management.RemoveFixtureType(typeA);
  BOOST_CHECK(management.GetTheatre().FixtureTypes().empty());
}

BOOST_AUTO_TEST_CASE(RemoveUsedFixtureType) {
  Management management;
  FixtureType &fixtureType =
      management.GetTheatre().AddFixtureType(StockFixture::Light1Ch);
  management.RootFolder().Add(fixtureType);
  Fixture &fixture = management.GetTheatre().AddFixture(fixtureType);
  FixtureControl &control =
      management.AddFixtureControl(fixture, management.RootFolder());
  SourceValue &value = management.AddSourceValue(control, 0);
  value.A().SetValue(ControlValue::Max());

  management.RemoveFixtureType(fixtureType);
  BOOST_CHECK(management.GetTheatre().FixtureTypes().empty());
  BOOST_CHECK(management.GetTheatre().Fixtures().empty());
  BOOST_CHECK(management.Controllables().empty());
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
