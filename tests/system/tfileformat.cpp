#include "theatre/chase.h"
#include "theatre/fixture.h"
#include "theatre/fixturecontrol.h"
#include "theatre/fixturegroup.h"
#include "theatre/fixturetype.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"
#include "theatre/theatre.h"
#include "theatre/timesequence.h"

#include "theatre/scenes/scene.h"

#include "theatre/effects/audioleveleffect.h"

#include "theatre/filters/automasterfilter.h"
#include "theatre/filters/rgbfilter.h"

#include "system/settings.h"

#include "uistate/uistate.h"

#include "system/reader.h"
#include "system/writer.h"

#include <boost/test/unit_test.hpp>

#include <filesystem>
#include <memory>

using namespace glight::theatre;
using glight::system::ObservingPtr;
using glight::system::TrackablePtr;

namespace {
void FillManagement(Management &management) {
  Folder &root = management.RootFolder();
  root.SetName("The root folder");
  Folder &subFolder = management.AddFolder(root, "A subfolder");

  // Use the RGB_ADJ_6CH fixture to test storing macro parameters
  root.Add(management.GetTheatre().AddFixtureTypePtr(StockFixture::Rgb));
  // Use the AyraTDCSunrise fixture to test rotation parameters:
  root.Add(
      management.GetTheatre().AddFixtureTypePtr(StockFixture::AyraTDCSunrise));

  // Make a group with the two fixtures
  management.AddFixtureGroup(root, "Ayra and ADJ");

  ObservingPtr<FixtureType> ft =
      management.GetTheatre().AddFixtureTypePtr(StockFixture::Rgbw);
  root.Add(ft);
  Fixture &f = *management.GetTheatre().AddFixture(ft->Modes().front());
  FixtureControl &fc = *management.AddFixtureControlPtr(f, subFolder);
  fc.SetName("Control for RGBW fixture");
  fc.AddFilter(std::make_unique<RgbFilter>());
  fc.AddFilter(std::make_unique<AutoMasterFilter>());
  management.AddSourceValue(fc, 0);
  management.AddSourceValue(fc, 1);
  management.AddSourceValue(fc, 2).A().SetValue(ControlValue::Max());
  management.AddSourceValue(fc, 3);
  ObservingPtr<FixtureControl> found_fc =
      management.GetFixtureControl(*management.GetTheatre().Fixtures()[0]);
  FolderObject &path_fc = management.GetObjectFromPath(
      "The root folder/A subfolder/Control for RGBW fixture");
  BOOST_CHECK(found_fc.Get() == &path_fc);

  ObservingPtr<PresetCollection> a = management.AddPresetCollectionPtr();
  a->SetName("A preset collection");
  subFolder.Add(a);
  a->AddPresetValue(fc, 0).SetValue(ControlValue::Max() / 2);
  a->AddPresetValue(fc, 3).SetValue(ControlValue::Max());
  management.AddSourceValue(*a, 0).A().SetValue(ControlValue(42));

  ObservingPtr<PresetCollection> b = management.AddPresetCollectionPtr();
  b->SetName("Second preset collection");
  subFolder.Add(b);
  b->AddPresetValue(fc, 0).SetValue(ControlValue(12));
  b->AddPresetValue(fc, 3).SetValue(ControlValue(13));
  management.AddSourceValue(*b, 0);

  ObservingPtr<Chase> chase = management.AddChasePtr();
  chase->SetName("A chase");
  subFolder.Add(chase);
  chase->GetSequence().Add(*a, 0);
  chase->GetSequence().Add(*b, 0);
  management.AddSourceValue(*chase, 0);

  ObservingPtr<TimeSequence> timeSequence = management.AddTimeSequencePtr();
  timeSequence->SetName("A time sequence");
  subFolder.Add(timeSequence);
  timeSequence->AddStep(*chase, 0);
  timeSequence->AddStep(*b, 0);
  management.AddSourceValue(*timeSequence, 0);

  Folder &effectFolder = management.AddFolder(root, "Effect folder");
  std::unique_ptr<AudioLevelEffect> effectPtr(new AudioLevelEffect());
  ObservingPtr<Effect> effect = management.AddEffectPtr(std::move(effectPtr));
  effect->SetName("An audio effect");
  effectFolder.Add(effect);
  effect->AddConnection(*a, 0);
  effect->AddConnection(fc, 1);

  ObservingPtr<Scene> scene = management.AddScenePtr(true);
  scene->SetName("Almost a movie");
  scene->SetAudioFile("A flac music file.flac");
  glight::theatre::KeySceneItem *key = scene->AddKeySceneItem(250.0);
  key->SetDurationInMS(500.0);
  key->SetLevel(KeySceneLevel::Beat);
  glight::theatre::BlackoutSceneItem &blackout = scene->AddBlackoutItem(250.0);
  blackout.SetFadeSpeed(2.0);
  blackout.SetOperation(BlackoutOperation::Blackout);
  glight::theatre::BlackoutSceneItem &restore = scene->AddBlackoutItem(250.0);
  restore.SetFadeSpeed(0.0);
  restore.SetOperation(BlackoutOperation::Restore);

  BOOST_CHECK(!management.HasCycle());
}

void CheckEqual(const FixtureModeFunction &a, const FixtureModeFunction &b) {
  BOOST_REQUIRE(a.Type() == b.Type());
  BOOST_CHECK_EQUAL(a.DmxOffset(), b.DmxOffset());
  BOOST_CHECK(a.FineChannelOffset() == b.FineChannelOffset());
  BOOST_CHECK_EQUAL(a.Shape(), b.Shape());

  switch (a.Type()) {
    case FunctionType::ColorMacro: {
      const ColorRangeParameters &a_pars = a.GetColorRangeParameters();
      const ColorRangeParameters &b_pars = b.GetColorRangeParameters();
      BOOST_REQUIRE_EQUAL(a_pars.GetRanges().size(), b_pars.GetRanges().size());
      for (size_t i = 0; i != a_pars.GetRanges().size(); ++i) {
        const ColorRangeParameters::Range &a_range = a_pars.GetRanges()[i];
        const ColorRangeParameters::Range &b_range = b_pars.GetRanges()[i];
        BOOST_CHECK(a_range.color == b_range.color);
        BOOST_CHECK_EQUAL(a_range.input_max, b_range.input_max);
        BOOST_CHECK_EQUAL(a_range.input_min, b_range.input_min);
      }
    } break;
    case FunctionType::RotationSpeed: {
      const RotationSpeedParameters &a_pars = a.GetRotationSpeedParameters();
      const RotationSpeedParameters &b_pars = b.GetRotationSpeedParameters();
      BOOST_REQUIRE_EQUAL(a_pars.GetRanges().size(), b_pars.GetRanges().size());
      for (size_t i = 0; i != a_pars.GetRanges().size(); ++i) {
        const RotationSpeedParameters::Range &a_range = a_pars.GetRanges()[i];
        const RotationSpeedParameters::Range &b_range = b_pars.GetRanges()[i];
        BOOST_CHECK_EQUAL(a_range.input_min, b_range.input_min);
        BOOST_CHECK_EQUAL(a_range.input_max, b_range.input_max);
        BOOST_CHECK_EQUAL(a_range.speed_min, b_range.speed_min);
        BOOST_CHECK_EQUAL(a_range.speed_max, b_range.speed_max);
      }
    } break;
    default:
      break;
  }
  // TODO parameters
}

void CheckEqual(const Management &a, const Management &b) {
  BOOST_CHECK_EQUAL(a.RootFolder().Name(), b.RootFolder().Name());
  BOOST_CHECK_EQUAL(a.RootFolder().Children()[0]->Name(),
                    b.RootFolder().Children()[0]->Name());

  BOOST_REQUIRE_EQUAL(a.GetTheatre().FixtureTypes().size(),
                      b.GetTheatre().FixtureTypes().size());
  for (size_t i = 0; i != a.GetTheatre().FixtureTypes().size(); ++i) {
    const FixtureType &a_t = *a.GetTheatre().FixtureTypes()[i];
    const FixtureType &b_t = *b.GetTheatre().FixtureTypes()[i];
    BOOST_CHECK(a_t.GetFixtureClass() == b_t.GetFixtureClass());
    BOOST_REQUIRE_EQUAL(a_t.Modes().size(), b_t.Modes().size());
    for (size_t m = 0; m != a_t.Modes().size(); ++m) {
      const FixtureMode &mode_a = a_t.Modes()[m];
      const FixtureMode &mode_b = b_t.Modes()[m];
      BOOST_REQUIRE_EQUAL(mode_a.Functions().size(), mode_b.Functions().size());
      for (size_t j = 0; j != mode_a.Functions().size(); ++j) {
        CheckEqual(mode_a.Functions()[j], mode_b.Functions()[j]);
      }
    }
  }

  BOOST_REQUIRE_EQUAL(a.GetTheatre().Fixtures().size(),
                      b.GetTheatre().Fixtures().size());

  const Fixture &a_fixture = *a.GetTheatre().Fixtures()[0];
  ObservingPtr<FixtureControl> a_fixture_control =
      a.GetFixtureControl(a_fixture);
  BOOST_CHECK_EQUAL(a_fixture_control->Name(), "Control for RGBW fixture");
  BOOST_CHECK_EQUAL(a_fixture_control->NInputs(),
                    3);  // rgb filter will make it 3
  BOOST_CHECK_EQUAL(a_fixture_control->NOutputs(), 0);
  BOOST_CHECK_EQUAL(
      a.GetFixtureControl(a_fixture).Get(),
      &a.GetObjectFromPath(
          "The root folder/A subfolder/Control for RGBW fixture"));

  BOOST_CHECK_EQUAL(a.GetSourceValue(*a_fixture_control, 0)->A().Value().UInt(),
                    0);
  BOOST_CHECK_EQUAL(a.GetSourceValue(*a_fixture_control, 1)->A().Value().UInt(),
                    0);
  BOOST_CHECK_EQUAL(a.GetSourceValue(*a_fixture_control, 2)->A().Value().UInt(),
                    ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(a.GetSourceValue(*a_fixture_control, 3)->A().Value().UInt(),
                    0);

  BOOST_REQUIRE_EQUAL(a.FixtureGroups().size(), b.FixtureGroups().size());
  for (size_t i = 0; i != a.FixtureGroups().size(); ++i) {
    BOOST_CHECK_EQUAL(a.FixtureGroups()[i]->Size(),
                      b.FixtureGroups()[i]->Size());
    const std::vector<ObservingPtr<Fixture>> &fixtures =
        a.FixtureGroups()[i]->Fixtures();
    for (const ObservingPtr<Fixture> &fixture_a : fixtures) {
      ObservingPtr<Fixture> fixture_b =
          b.GetTheatre().GetFixturePtr(fixture_a->Name());
      BOOST_CHECK(b.FixtureGroups()[i]->Contains(fixture_b));
    }
  }

  const PresetCollection &readCollection =
      static_cast<const PresetCollection &>(a.GetObjectFromPath(
          "The root folder/A subfolder/A preset collection"));
  BOOST_CHECK_EQUAL(readCollection.Name(), "A preset collection");
  BOOST_CHECK_EQUAL(readCollection.NInputs(), 1);
  BOOST_CHECK_EQUAL(readCollection.NOutputs(), 2);
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[0]->Value().UInt(),
                    ControlValue::MaxUInt() / 2);
  BOOST_CHECK_EQUAL(&readCollection.PresetValues()[0]->GetControllable(),
                    a_fixture_control.Get());
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[0]->InputIndex(), 0);
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[1]->Value().UInt(),
                    ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(&readCollection.PresetValues()[1]->GetControllable(),
                    a_fixture_control.Get());
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[1]->InputIndex(), 3);
  BOOST_CHECK_NE(a.GetSourceValue(readCollection, 0), nullptr);
  BOOST_CHECK_EQUAL(&a.GetSourceValue(readCollection, 0)->GetControllable(),
                    &readCollection);
  BOOST_CHECK_EQUAL(a.GetSourceValue(readCollection, 0)->InputIndex(), 0);
  BOOST_CHECK_EQUAL(a.GetSourceValue(readCollection, 0)->A().Value().UInt(),
                    42);

  const Chase &readChase = static_cast<const Chase &>(
      a.GetObjectFromPath("The root folder/A subfolder/A chase"));
  BOOST_CHECK_EQUAL(readChase.GetSequence().Size(), 2);
  BOOST_CHECK_EQUAL(readChase.GetSequence().List()[0].GetControllable(),
                    &readCollection);
  BOOST_CHECK_EQUAL(readChase.GetSequence().List()[0].InputIndex(), 0);

  const AudioLevelEffect *readEffect = dynamic_cast<const AudioLevelEffect *>(
      &a.GetObjectFromPath("The root folder/Effect folder/An audio effect"));
  BOOST_CHECK_NE(readEffect, nullptr);
  BOOST_CHECK_EQUAL(readEffect->Connections().size(), 2);
  BOOST_CHECK_EQUAL(readEffect->Connections()[0].first, &readCollection);
  BOOST_CHECK_EQUAL(readEffect->Connections()[0].second, 0);
  BOOST_CHECK_EQUAL(readEffect->Connections()[1].first,
                    a_fixture_control.Get());
  BOOST_CHECK_EQUAL(readEffect->Connections()[1].second, 1);

  BOOST_REQUIRE_EQUAL(a.Controllables().size(), b.Controllables().size());
  for (size_t controllable_index = 0;
       controllable_index != a.Controllables().size(); ++controllable_index) {
    const Controllable &controllable_a = *a.Controllables()[controllable_index];
    const Controllable &controllable_b = *b.Controllables()[controllable_index];
    BOOST_CHECK_EQUAL(controllable_a.FullPath(), controllable_b.FullPath());
    BOOST_CHECK_EQUAL(controllable_a.NInputs(), controllable_b.NInputs());
    BOOST_CHECK_EQUAL(controllable_a.NOutputs(), controllable_b.NOutputs());
    if (const Scene *scene_a = dynamic_cast<const Scene *>(&controllable_a);
        scene_a) {
      const Scene *scene_b = dynamic_cast<const Scene *>(&controllable_b);
      BOOST_CHECK(scene_b);
      BOOST_CHECK_EQUAL(scene_a->SceneItems().size(),
                        scene_b->SceneItems().size());
      auto iterator_b = scene_b->SceneItems().begin();
      for (const auto &pair_a : scene_a->SceneItems()) {
        BOOST_CHECK(typeid(pair_a.second.get()) ==
                    typeid(iterator_b->second.get()));
        if (BlackoutSceneItem *blackout_a =
                dynamic_cast<BlackoutSceneItem *>(pair_a.second.get());
            blackout_a) {
          BlackoutSceneItem *blackout_b =
              dynamic_cast<BlackoutSceneItem *>(iterator_b->second.get());
          BOOST_CHECK(blackout_b);
          BOOST_CHECK(blackout_a->Operation() == blackout_b->Operation());
          BOOST_CHECK_CLOSE_FRACTION(blackout_a->FadeSpeed(),
                                     blackout_b->FadeSpeed(), 1e-5);
        }
        ++iterator_b;
      }
    }
  }
}

}  // namespace

BOOST_AUTO_TEST_SUITE(file_format)

BOOST_AUTO_TEST_CASE(ReadAndWrite) {
  glight::system::Settings settings;
  Management write_management(settings);
  FillManagement(write_management);

  glight::uistate::UIState guiState;
  std::vector<std::unique_ptr<glight::uistate::FaderSetState>> &setups =
      guiState.FaderSets();
  std::unique_ptr<glight::uistate::FaderSetState> &setup =
      setups.emplace_back(std::make_unique<glight::uistate::FaderSetState>());
  setup->name = "testfader";
  glight::uistate::FaderState &state = *setup->faders.emplace_back(
      std::make_unique<glight::uistate::FaderState>());
  state.SetSourceValues({write_management.SourceValues()[0].get()});

  glight::system::Write("tmp-testfileformat.gshow", write_management,
                        &guiState);
  BOOST_CHECK(std::filesystem::exists("tmp-testfileformat.gshow"));
  std::ostringstream stream;
  glight::system::Write(stream, write_management, &guiState);

  Management read_management(settings);
  //
  // Read and check if the result is correct
  //
  BOOST_TEST_CHECKPOINT("Start of reading");
  glight::uistate::UIState resultGuiState;
  std::istringstream istream(stream.str());
  glight::system::Read(istream, read_management, &resultGuiState);

  CheckEqual(read_management, write_management);
}

BOOST_AUTO_TEST_SUITE_END()
