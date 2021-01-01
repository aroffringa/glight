#include "../theatre/chase.h"
#include "../theatre/fixture.h"
#include "../theatre/fixturecontrol.h"
#include "../theatre/folder.h"
#include "../theatre/management.h"
#include "../theatre/presetcollection.h"
#include "../theatre/theatre.h"
#include "../theatre/timesequence.h"

#include "../theatre/effects/audioleveleffect.h"

#include "../system/reader.h"
#include "../system/writer.h"

#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_SUITE(file_format)

BOOST_AUTO_TEST_CASE(ReadAndWrite) {
  Management management;
  Folder &root = management.RootFolder();
  root.SetName("The root folder");
  Folder &subFolder = management.AddFolder(root, "A subfolder");

  FixtureType &ft =
      management.Theatre().AddFixtureType(FixtureType::RGBWLight4Ch);
  root.Add(ft);
  Fixture &f = management.Theatre().AddFixture(ft);
  FixtureControl &fc = management.AddFixtureControl(f, subFolder);
  fc.SetName("Control for RGBW fixture");
  management.AddSourceValue(fc, 0);
  management.AddSourceValue(fc, 1);
  management.AddSourceValue(fc, 2).Preset().SetValue(ControlValue::MaxUInt());
  management.AddSourceValue(fc, 3);
  BOOST_CHECK_EQUAL(
      &management.GetFixtureControl(*management.Theatre().Fixtures()[0]),
      &management.GetObjectFromPath(
          "The root folder/A subfolder/Control for RGBW fixture"));

  PresetCollection &a = management.AddPresetCollection();
  a.SetName("A preset collection");
  subFolder.Add(a);
  a.AddPresetValue(fc, 0).SetValue(ControlValue::MaxUInt() / 2);
  a.AddPresetValue(fc, 3).SetValue(ControlValue::MaxUInt());
  management.AddSourceValue(a, 0).Preset().SetValue(42);

  PresetCollection &b = management.AddPresetCollection();
  b.SetName("Second preset collection");
  subFolder.Add(b);
  b.AddPresetValue(fc, 0).SetValue(12);
  b.AddPresetValue(fc, 3).SetValue(13);
  management.AddSourceValue(b, 0);

  Chase &chase = management.AddChase();
  chase.SetName("A chase");
  subFolder.Add(chase);
  chase.Sequence().Add(a, 0);
  chase.Sequence().Add(b, 0);
  management.AddSourceValue(chase, 0);

  TimeSequence &timeSequence = management.AddTimeSequence();
  timeSequence.SetName("A time sequence");
  subFolder.Add(timeSequence);
  timeSequence.AddStep(chase, 0);
  timeSequence.AddStep(b, 0);
  management.AddSourceValue(timeSequence, 0);

  Folder &effectFolder = management.AddFolder(root, "Effect folder");
  std::unique_ptr<AudioLevelEffect> effectPtr(new AudioLevelEffect());
  Effect &effect = management.AddEffect(std::move(effectPtr));
  effect.SetName("An audio effect");
  effectFolder.Add(effect);
  effect.AddConnection(a, 0);
  effect.AddConnection(fc, 1);

  BOOST_CHECK(!management.HasCycle());
  Writer writer(management);
  writer.Write("tmp-testfileformat.gshow");

  BOOST_CHECK(boost::filesystem::exists("tmp-testfileformat.gshow"));

  management.Clear();
  BOOST_CHECK_EQUAL(management.Controllables().size(), 0);
  BOOST_CHECK_EQUAL(management.Theatre().Fixtures().size(), 0);

  //
  // Read and check if the result is correct
  //
  BOOST_TEST_CHECKPOINT("Start of reading");
  Reader reader(management);
  reader.Read("tmp-testfileformat.gshow");
  // boost::filesystem::remove("tmp-testfileformat.gshow");
  // BOOST_CHECK(!boost::filesystem::exists("tmp-testfileformat.gshow"));

  BOOST_CHECK_EQUAL(management.RootFolder().Name(), "The root folder");
  BOOST_CHECK_EQUAL(management.RootFolder().Children()[0]->Name(),
                    "A subfolder");
  BOOST_CHECK_EQUAL(management.Theatre().Fixtures().size(), 1);

  Fixture &readFixture = *management.Theatre().Fixtures()[0];
  FixtureControl &readFixtureControl =
      management.GetFixtureControl(readFixture);
  BOOST_CHECK_EQUAL(readFixtureControl.Name(), "Control for RGBW fixture");
  BOOST_CHECK_EQUAL(readFixtureControl.NInputs(), 4);
  BOOST_CHECK_EQUAL(readFixtureControl.NOutputs(), 0);
  BOOST_CHECK_EQUAL(
      &management.GetFixtureControl(readFixture),
      &management.GetObjectFromPath(
          "The root folder/A subfolder/Control for RGBW fixture"));

  BOOST_CHECK_EQUAL(
      management.GetSourceValue(readFixtureControl, 0)->Preset().Value().UInt(),
      0);
  BOOST_CHECK_EQUAL(
      management.GetSourceValue(readFixtureControl, 1)->Preset().Value().UInt(),
      0);
  BOOST_CHECK_EQUAL(
      management.GetSourceValue(readFixtureControl, 2)->Preset().Value().UInt(),
      ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(
      management.GetSourceValue(readFixtureControl, 3)->Preset().Value().UInt(),
      0);

  PresetCollection &readCollection =
      static_cast<PresetCollection &>(management.GetObjectFromPath(
          "The root folder/A subfolder/A preset collection"));
  BOOST_CHECK_EQUAL(readCollection.Name(), "A preset collection");
  BOOST_CHECK_EQUAL(readCollection.NInputs(), 1);
  BOOST_CHECK_EQUAL(readCollection.NOutputs(), 2);
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[0]->Value().UInt(),
                    ControlValue::MaxUInt() / 2);
  BOOST_CHECK_EQUAL(&readCollection.PresetValues()[0]->Controllable(),
                    &readFixtureControl);
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[0]->InputIndex(), 0);
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[1]->Value().UInt(),
                    ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(&readCollection.PresetValues()[1]->Controllable(),
                    &readFixtureControl);
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[1]->InputIndex(), 3);
  BOOST_CHECK_NE(management.GetSourceValue(readCollection, 0), nullptr);
  BOOST_CHECK_EQUAL(
      &management.GetSourceValue(readCollection, 0)->Controllable(),
      &readCollection);
  BOOST_CHECK_EQUAL(
      management.GetSourceValue(readCollection, 0)->Preset().InputIndex(), 0);
  BOOST_CHECK_EQUAL(
      management.GetSourceValue(readCollection, 0)->Preset().Value().UInt(),
      42);

  Chase &readChase = static_cast<Chase &>(
      management.GetObjectFromPath("The root folder/A subfolder/A chase"));
  BOOST_CHECK_EQUAL(readChase.Sequence().Size(), 2);
  BOOST_CHECK_EQUAL(readChase.Sequence().List()[0].first, &readCollection);
  BOOST_CHECK_EQUAL(readChase.Sequence().List()[0].second, 0);

  AudioLevelEffect *readEffect =
      dynamic_cast<AudioLevelEffect *>(&management.GetObjectFromPath(
          "The root folder/Effect folder/An audio effect"));
  BOOST_CHECK_NE(readEffect, nullptr);
  BOOST_CHECK_EQUAL(readEffect->Connections().size(), 2);
  BOOST_CHECK_EQUAL(readEffect->Connections()[0].first, &readCollection);
  BOOST_CHECK_EQUAL(readEffect->Connections()[0].second, 0);
  BOOST_CHECK_EQUAL(readEffect->Connections()[1].first, &readFixtureControl);
  BOOST_CHECK_EQUAL(readEffect->Connections()[1].second, 1);
}

BOOST_AUTO_TEST_SUITE_END()
