#include "../theatre/chase.h"
#include "../theatre/fixture.h"
#include "../theatre/fixturecontrol.h"
#include "../theatre/folder.h"
#include "../theatre/management.h"
#include "../theatre/presetcollection.h"
#include "../theatre/theatre.h"
#include "../theatre/timesequence.h"

#include "../theatre/effects/audioleveleffect.h"

#include "../gui/guistate.h"

#include "../system/reader.h"
#include "../system/writer.h"

#include <boost/filesystem/operations.hpp>
#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;

namespace {
void FillManagement(Management &management) {
  Folder &root = management.RootFolder();
  root.SetName("The root folder");
  Folder &subFolder = management.AddFolder(root, "A subfolder");

  // Use the RGB_ADJ_6CH fixture to test storing macro parameters
  root.Add(management.GetTheatre().AddFixtureType(StockFixture::RGB_ADJ_6CH));
  // Use the AyraTDCSunrise fixture to test rotation parameters:
  root.Add(
      management.GetTheatre().AddFixtureType(StockFixture::AyraTDCSunrise));

  FixtureType &ft =
      management.GetTheatre().AddFixtureType(StockFixture::RGBWLight4Ch);
  root.Add(ft);
  Fixture &f = management.GetTheatre().AddFixture(ft);
  FixtureControl &fc = management.AddFixtureControl(f, subFolder);
  fc.SetName("Control for RGBW fixture");
  management.AddSourceValue(fc, 0);
  management.AddSourceValue(fc, 1);
  management.AddSourceValue(fc, 2).Preset().SetValue(ControlValue::MaxUInt());
  management.AddSourceValue(fc, 3);
  BOOST_CHECK_EQUAL(
      &management.GetFixtureControl(*management.GetTheatre().Fixtures()[0]),
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
}

void CheckEqual(const FixtureTypeFunction &a, const FixtureTypeFunction &b) {
  BOOST_REQUIRE(a.Type() == b.Type());
  BOOST_CHECK_EQUAL(a.DmxOffset(), b.DmxOffset());
  BOOST_CHECK_EQUAL(a.Is16Bit(), b.Is16Bit());
  BOOST_CHECK_EQUAL(a.Shape(), b.Shape());

  switch (a.Type()) {
    case FunctionType::ColorMacro: {
      const MacroParameters &a_pars = a.GetMacroParameters();
      const MacroParameters &b_pars = b.GetMacroParameters();
      BOOST_REQUIRE_EQUAL(a_pars.GetRanges().size(), b_pars.GetRanges().size());
      for (size_t i = 0; i != a_pars.GetRanges().size(); ++i) {
        const MacroParameters::Range &a_range = a_pars.GetRanges()[i];
        const MacroParameters::Range &b_range = b_pars.GetRanges()[i];
        BOOST_CHECK(a_range.color == b_range.color);
        BOOST_CHECK_EQUAL(a_range.input_max, b_range.input_max);
        BOOST_CHECK_EQUAL(a_range.input_min, b_range.input_min);
      }
    } break;
    case FunctionType::Rotation: {
      const RotationParameters &a_pars = a.GetRotationParameters();
      const RotationParameters &b_pars = b.GetRotationParameters();
      BOOST_REQUIRE_EQUAL(a_pars.GetRanges().size(), b_pars.GetRanges().size());
      for (size_t i = 0; i != a_pars.GetRanges().size(); ++i) {
        const RotationParameters::Range &a_range = a_pars.GetRanges()[i];
        const RotationParameters::Range &b_range = b_pars.GetRanges()[i];
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
    BOOST_REQUIRE_EQUAL(a_t.Functions().size(), b_t.Functions().size());
    for (size_t j = 0; j != a_t.Functions().size(); ++j) {
      CheckEqual(a_t.Functions()[j], b_t.Functions()[j]);
    }
  }

  BOOST_CHECK_EQUAL(a.GetTheatre().Fixtures().size(),
                    b.GetTheatre().Fixtures().size());

  const Fixture &a_fixture = *a.GetTheatre().Fixtures()[0];
  const FixtureControl &a_fixture_control = a.GetFixtureControl(a_fixture);
  BOOST_CHECK_EQUAL(a_fixture_control.Name(), "Control for RGBW fixture");
  BOOST_CHECK_EQUAL(a_fixture_control.NInputs(), 4);
  BOOST_CHECK_EQUAL(a_fixture_control.NOutputs(), 0);
  BOOST_CHECK_EQUAL(
      &a.GetFixtureControl(a_fixture),
      &a.GetObjectFromPath(
          "The root folder/A subfolder/Control for RGBW fixture"));

  BOOST_CHECK_EQUAL(
      a.GetSourceValue(a_fixture_control, 0)->Preset().Value().UInt(), 0);
  BOOST_CHECK_EQUAL(
      a.GetSourceValue(a_fixture_control, 1)->Preset().Value().UInt(), 0);
  BOOST_CHECK_EQUAL(
      a.GetSourceValue(a_fixture_control, 2)->Preset().Value().UInt(),
      ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(
      a.GetSourceValue(a_fixture_control, 3)->Preset().Value().UInt(), 0);

  const PresetCollection &readCollection =
      static_cast<const PresetCollection &>(a.GetObjectFromPath(
          "The root folder/A subfolder/A preset collection"));
  BOOST_CHECK_EQUAL(readCollection.Name(), "A preset collection");
  BOOST_CHECK_EQUAL(readCollection.NInputs(), 1);
  BOOST_CHECK_EQUAL(readCollection.NOutputs(), 2);
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[0]->Value().UInt(),
                    ControlValue::MaxUInt() / 2);
  BOOST_CHECK_EQUAL(&readCollection.PresetValues()[0]->Controllable(),
                    &a_fixture_control);
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[0]->InputIndex(), 0);
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[1]->Value().UInt(),
                    ControlValue::MaxUInt());
  BOOST_CHECK_EQUAL(&readCollection.PresetValues()[1]->Controllable(),
                    &a_fixture_control);
  BOOST_CHECK_EQUAL(readCollection.PresetValues()[1]->InputIndex(), 3);
  BOOST_CHECK_NE(a.GetSourceValue(readCollection, 0), nullptr);
  BOOST_CHECK_EQUAL(&a.GetSourceValue(readCollection, 0)->GetControllable(),
                    &readCollection);
  BOOST_CHECK_EQUAL(a.GetSourceValue(readCollection, 0)->Preset().InputIndex(),
                    0);
  BOOST_CHECK_EQUAL(
      a.GetSourceValue(readCollection, 0)->Preset().Value().UInt(), 42);

  const Chase &readChase = static_cast<const Chase &>(
      a.GetObjectFromPath("The root folder/A subfolder/A chase"));
  BOOST_CHECK_EQUAL(readChase.Sequence().Size(), 2);
  BOOST_CHECK_EQUAL(readChase.Sequence().List()[0].first, &readCollection);
  BOOST_CHECK_EQUAL(readChase.Sequence().List()[0].second, 0);

  const AudioLevelEffect *readEffect = dynamic_cast<const AudioLevelEffect *>(
      &a.GetObjectFromPath("The root folder/Effect folder/An audio effect"));
  BOOST_CHECK_NE(readEffect, nullptr);
  BOOST_CHECK_EQUAL(readEffect->Connections().size(), 2);
  BOOST_CHECK_EQUAL(readEffect->Connections()[0].first, &readCollection);
  BOOST_CHECK_EQUAL(readEffect->Connections()[0].second, 0);
  BOOST_CHECK_EQUAL(readEffect->Connections()[1].first, &a_fixture_control);
  BOOST_CHECK_EQUAL(readEffect->Connections()[1].second, 1);
}

}  // namespace

BOOST_AUTO_TEST_SUITE(file_format)

BOOST_AUTO_TEST_CASE(ReadAndWrite) {
  Management write_management;
  FillManagement(write_management);

  glight::gui::GUIState guiState;
  std::vector<std::unique_ptr<glight::gui::FaderSetupState>> &setups =
      guiState.FaderSetups();
  std::unique_ptr<glight::gui::FaderSetupState> &setup =
      setups.emplace_back(std::make_unique<glight::gui::FaderSetupState>());
  setup->name = "testfader";
  glight::gui::FaderState &state = setup->faders.emplace_back();
  state.SetSourceValue(write_management.SourceValues()[0].get());

  glight::system::Write("tmp-testfileformat.gshow", write_management,
                        &guiState);
  BOOST_CHECK(boost::filesystem::exists("tmp-testfileformat.gshow"));
  std::ostringstream stream;
  glight::system::Write(stream, write_management, &guiState);

  Management read_management;
  //
  // Read and check if the result is correct
  //
  BOOST_TEST_CHECKPOINT("Start of reading");
  glight::gui::GUIState resultGuiState;
  std::istringstream istream(stream.str());
  glight::system::Read(istream, read_management, &resultGuiState);

  CheckEqual(read_management, write_management);
}

BOOST_AUTO_TEST_SUITE_END()
