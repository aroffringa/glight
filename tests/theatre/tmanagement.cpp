#include "theatre/chase.h"
#include "theatre/fixturecontrol.h"
#include "theatre/fixturetype.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"
#include "theatre/sourcevalue.h"
#include "theatre/theatre.h"
#include "theatre/timesequence.h"

#include "theatre/effects/fadeeffect.h"

#include "system/settings.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;
using glight::system::ObservingPtr;

BOOST_AUTO_TEST_SUITE(management)

BOOST_AUTO_TEST_CASE(Destruct) {
  const glight::system::Settings settings;
  Management management(settings);
  management.Run();
  BOOST_CHECK_THROW(management.Run(), std::exception);
  management.StartBeatFinder();
}

BOOST_AUTO_TEST_CASE(RemoveObject) {
  const glight::system::Settings settings;
  Management management(settings);
  std::unique_ptr<FadeEffect> effectPtr(new FadeEffect());
  effectPtr->SetName("effect");
  BOOST_CHECK(management.RootFolder().Children().empty());
  Folder &folder = management.AddFolder(management.RootFolder(), "folder");
  Effect &effect = *management.AddEffectPtr(std::move(effectPtr), folder);
  management.AddSourceValue(effect, 0);
  ObservingPtr<Chase> chase = management.AddChasePtr();
  chase->SetName("chase");
  folder.Add(chase);
  management.AddSourceValue(*chase, 0);
  BOOST_CHECK_EQUAL(management.RootFolder().Children().size(), 1);
  BOOST_CHECK_EQUAL(folder.Children().size(), 2);
  BOOST_CHECK(!management.SourceValues().empty());

  management.RemoveObject(folder);

  BOOST_CHECK(management.RootFolder().Children().empty());
  BOOST_CHECK(management.SourceValues().empty());
}

BOOST_AUTO_TEST_CASE(GetSpecificControllables) {
  const glight::system::Settings settings;
  Management management(settings);
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
  const glight::system::Settings settings;
  Management management(settings);

  ObservingPtr<FixtureType> typeA =
      management.GetTheatre().AddFixtureTypePtr(StockFixture::Light);
  management.RootFolder().Add(typeA);
  BOOST_CHECK(typeA);

  ObservingPtr<FixtureType> typeB =
      management.GetTheatre().AddFixtureTypePtr(StockFixture::Rgb);
  management.RootFolder().Add(typeB);
  BOOST_CHECK(typeB);

  ObservingPtr<FixtureType> typeC =
      management.GetTheatre().AddFixtureTypePtr(StockFixture::Rgba);
  management.RootFolder().Add(typeC);
  BOOST_CHECK(typeC);

  management.RemoveFixtureType(*typeB);
  BOOST_CHECK(typeC);
  management.RemoveFixtureType(*typeC);
  management.RemoveFixtureType(*typeA);
  BOOST_CHECK(management.GetTheatre().FixtureTypes().empty());
}

BOOST_AUTO_TEST_CASE(RemoveUsedFixtureType) {
  const glight::system::Settings settings;
  Management management(settings);
  ObservingPtr<FixtureType> fixtureType =
      management.GetTheatre().AddFixtureTypePtr(StockFixture::Light);
  management.RootFolder().Add(fixtureType);
  Fixture &fixture =
      *management.GetTheatre().AddFixture(fixtureType->Modes().front());
  FixtureControl &control =
      *management.AddFixtureControlPtr(fixture, management.RootFolder());
  SourceValue &value = management.AddSourceValue(control, 0);
  value.A().SetValue(ControlValue::Max());

  management.RemoveFixtureType(*fixtureType);
  BOOST_CHECK(management.GetTheatre().FixtureTypes().empty());
  BOOST_CHECK(management.GetTheatre().Fixtures().empty());
  BOOST_CHECK(management.Controllables().empty());
}

BOOST_AUTO_TEST_CASE(HasCycles) {
  const glight::system::Settings settings;
  Management management(settings);
  BOOST_CHECK_EQUAL(management.HasCycle(), false);

  std::unique_ptr<FadeEffect> effectPtr(new FadeEffect());
  Effect &effect = *management.AddEffectPtr(std::move(effectPtr));
  BOOST_CHECK_EQUAL(management.HasCycle(), false);
  effect.AddConnection(effect, 0);
  BOOST_CHECK_EQUAL(management.HasCycle(), true);
  effect.RemoveConnection(0);
  BOOST_CHECK_EQUAL(management.HasCycle(), false);

  TimeSequence &timeSeq = *management.AddTimeSequencePtr();
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

  PresetCollection &collection = *management.AddPresetCollectionPtr();
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
