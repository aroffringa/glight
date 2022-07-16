#include <fstream>

#include "writer.h"

#include "../theatre/properties/propertyset.h"

#include "../theatre/chase.h"
#include "../theatre/controllable.h"
#include "../theatre/effect.h"
#include "../theatre/fixture.h"
#include "../theatre/fixturecontrol.h"
#include "../theatre/fixturefunction.h"
#include "../theatre/folder.h"
#include "../theatre/presetvalue.h"
#include "../theatre/show.h"
#include "../theatre/theatre.h"
#include "../theatre/timesequence.h"

#include "../gui/guistate.h"

namespace glight::system {
namespace {

using namespace glight::theatre;

struct WriteState {
  WriteState(Management &management_) : management(management_) {}
  json::JsonWriter writer;
  std::set<const Controllable *> controllablesWritten;
  std::map<const Folder *, size_t> folderIds;
  Management &management;
  gui::GUIState *guiState = nullptr;
};

void writeControllable(WriteState &state, const Controllable &controllable);

void writeFolders(WriteState &state) {
  state.writer.StartArray("folders");
  for (size_t i = 0; i != state.management.Folders().size(); ++i) {
    const Folder &folder = *state.management.Folders()[i];
    state.folderIds.emplace(&folder, i);
    state.writer.StartObject();
    state.writer.Number("id", i);
    state.writer.String("name", folder.Name());
    if (!folder.IsRoot())
      state.writer.Number("parent",
                          state.folderIds.find(&folder.Parent())->second);
    state.writer.EndObject();
  }
  state.writer.EndArray();
}

void writeNameAttributes(WriteState &state, const NamedObject &obj) {
  state.writer.String("name", obj.Name());
}

void writeFolderAttributes(WriteState &state, const FolderObject &obj) {
  writeNameAttributes(state, obj);
  state.writer.Number("parent", state.folderIds.find(&obj.Parent())->second);
}

void writeDmxChannel(WriteState &state, const DmxChannel &dmxChannel) {
  state.writer.StartObject("dmx-channel");
  state.writer.Number("universe", dmxChannel.Universe());
  state.writer.Number("channel", dmxChannel.Channel());
  state.writer.String("default-mix-style",
                      ToString(dmxChannel.DefaultMixStyle()));
  state.writer.EndObject();
}

void writeFixtureFunction(WriteState &state,
                          const FixtureFunction &fixtureFunction) {
  state.writer.StartObject();
  state.writer.String("name", fixtureFunction.Name());
  state.writer.String("type", ToString(fixtureFunction.Type()));
  writeDmxChannel(state, fixtureFunction.FirstChannel());
  state.writer.EndObject();
}

void writeFixture(WriteState &state, const Fixture &fixture) {
  state.writer.StartObject();
  writeNameAttributes(state, fixture);
  state.writer.String("type", fixture.Type().Name());
  state.writer.Number("position-x", fixture.GetPosition().X());
  state.writer.Number("position-y", fixture.GetPosition().Y());
  state.writer.String("symbol", fixture.Symbol().Name());
  const std::vector<std::unique_ptr<FixtureFunction>> &functions =
      fixture.Functions();
  state.writer.StartArray("functions");
  for (const std::unique_ptr<FixtureFunction> &ff : functions)
    writeFixtureFunction(state, *ff);
  state.writer.EndArray();   // functions
  state.writer.EndObject();  // fixture
}

void witeMacroParameters(WriteState &state, const MacroParameters &pars) {
  state.writer.StartObject("parameters");
  state.writer.StartArray("ranges");
  for (const MacroParameters::Range &range : pars.GetRanges()) {
    state.writer.StartObject();
    state.writer.Number("input-min", range.input_min);
    state.writer.Number("input-max", range.input_max);
    state.writer.Number("red", range.color->Red());
    state.writer.Number("green", range.color->Green());
    state.writer.Number("blue", range.color->Blue());
    state.writer.EndObject();
  }
  state.writer.EndArray();
  state.writer.EndObject();
}

void writeRotationParameters(WriteState &state,
                             const RotationParameters &pars) {}

void writeFixtureTypeFunction(WriteState &state,
                              const FixtureTypeFunction &function) {
  state.writer.StartObject();
  state.writer.String("type", ToString(function.Type()));
  state.writer.Number("dmx-offset", function.DmxOffset());
  state.writer.Boolean("is-16-bit", function.Is16Bit());
  state.writer.Number("shape", function.Shape());
  switch (function.Type()) {
    case FunctionType::ColorMacro:
      witeMacroParameters(state, function.GetMacroParameters());
      break;
    case FunctionType::Rotation:
      writeRotationParameters(state, function.GetRotationParameters());
      break;
    default:
      break;
  }
  state.writer.EndObject();
}

void writeFixtureType(WriteState &state, const FixtureType &fixtureType) {
  state.writer.StartObject();
  writeFolderAttributes(state, fixtureType);
  state.writer.String("fixture-class",
                      FixtureType::ClassName(fixtureType.GetFixtureClass()));
  state.writer.Number("shape-count", fixtureType.ShapeCount());
  state.writer.StartArray("functions");
  for (const FixtureTypeFunction &f : fixtureType.Functions()) {
    writeFixtureTypeFunction(state, f);
  }
  state.writer.EndArray();  // functions
  state.writer.EndObject();
}

void writePresetValue(WriteState &state, const PresetValue &presetValue) {
  writeControllable(state, presetValue.Controllable());

  state.writer.StartObject();
  state.writer.String("controllable-ref", presetValue.Controllable().Name());
  state.writer.Number("input-index", presetValue.InputIndex());
  state.writer.Number("folder",
                      state.folderIds[&presetValue.Controllable().Parent()]);
  state.writer.Number("value", presetValue.Value().UInt());
  state.writer.EndObject();
}

void writePresetCollection(WriteState &state,
                           const class PresetCollection &presetCollection) {
  const std::vector<std::unique_ptr<PresetValue>> &values =
      presetCollection.PresetValues();
  for (const std::unique_ptr<PresetValue> &pv : values)
    writeControllable(state, pv->Controllable());

  state.writer.StartObject();
  state.writer.String("type", "preset-collection");
  writeFolderAttributes(state, presetCollection);
  state.writer.StartArray("values");
  for (const std::unique_ptr<PresetValue> &pv : values)
    writePresetValue(state, *pv);
  state.writer.EndArray();
  state.writer.EndObject();
}

void writeFixtureControl(WriteState &state, const FixtureControl &control) {
  state.writer.StartObject();
  state.writer.String("type", "fixture-control");
  writeFolderAttributes(state, control);
  state.writer.String("fixture-ref", control.Fixture().Name());
  state.writer.EndObject();
}

void writeTrigger(WriteState &state, const Trigger &trigger) {
  state.writer.StartObject("trigger");
  state.writer.String("type", ToString(trigger.Type()));
  state.writer.Number("delay-in-ms", trigger.DelayInMs());
  state.writer.Number("delay-in-beats", trigger.DelayInBeats());
  state.writer.Number("delay-in-syncs", trigger.DelayInSyncs());
  state.writer.EndObject();
}

void writeTransition(WriteState &state, const Transition &transition) {
  state.writer.StartObject("transition");
  state.writer.Number("length-in-ms", transition.LengthInMs());
  state.writer.String("type", ToString(transition.Type()));
  state.writer.EndObject();
}

void writeSequence(WriteState &state, const Sequence &sequence) {
  state.writer.StartObject("sequence");
  state.writer.StartArray("inputs");
  for (const std::pair<Controllable *, size_t> &input : sequence.List()) {
    state.writer.StartObject();
    state.writer.Number("input-index", input.second);
    state.writer.Number("folder", state.folderIds[&input.first->Parent()]);
    state.writer.String("name", input.first->Name());
    state.writer.EndObject();
  }
  state.writer.EndArray();
  state.writer.EndObject();
}

void writeChase(WriteState &state, const Chase &chase) {
  const std::vector<std::pair<Controllable *, size_t>> &list =
      chase.Sequence().List();
  for (const std::pair<Controllable *, size_t> &input : list)
    writeControllable(state, *input.first);

  state.writer.StartObject();
  state.writer.String("type", "chase");
  writeFolderAttributes(state, chase);
  writeTrigger(state, chase.Trigger());
  writeTransition(state, chase.Transition());
  writeSequence(state, chase.Sequence());
  state.writer.EndObject();
}

void writeTimeSequence(WriteState &state, const TimeSequence &timeSequence) {
  const std::vector<std::pair<Controllable *, size_t>> &list =
      timeSequence.Sequence().List();
  for (const std::pair<Controllable *, size_t> &input : list)
    writeControllable(state, *input.first);

  state.writer.StartObject();
  state.writer.String("type", "time-sequence");
  writeFolderAttributes(state, timeSequence);
  state.writer.Boolean("sustain", timeSequence.Sustain());
  state.writer.Number("repeat-count", timeSequence.RepeatCount());
  writeSequence(state, timeSequence.Sequence());
  state.writer.StartArray("steps");
  for (size_t i = 0; i != timeSequence.Size(); ++i) {
    const TimeSequence::Step &step = timeSequence.GetStep(i);
    state.writer.StartObject();
    writeTrigger(state, step.trigger);
    writeTransition(state, step.transition);
    state.writer.EndObject();
  }
  state.writer.EndArray();   // steps
  state.writer.EndObject();  // time-sequence
}

void writeEffect(WriteState &state, const Effect &effect) {
  if (state.controllablesWritten.count(&effect) == 0) {
    for (const std::pair<Controllable *, size_t> &c : effect.Connections())
      writeControllable(state, *c.first);

    state.writer.StartObject();
    state.writer.String("type", "effect");
    writeFolderAttributes(state, effect);
    state.writer.String("effect_type", effect.TypeToName(effect.GetType()));
    std::unique_ptr<PropertySet> ps = PropertySet::Make(effect);

    // the number and name of the effect controls are implied from the
    // effect type, so do not require to be stored.

    state.writer.StartArray("properties");
    for (const Property &p : *ps) {
      state.writer.StartObject();
      state.writer.String("name", p.Name());
      switch (p.GetType()) {
        case Property::Choice:
          state.writer.String("value", ps->GetChoice(p));
          break;
        case Property::ControlValue:
          state.writer.Number("value", ps->GetControlValue(p));
          break;
        case Property::Duration:
          state.writer.Number("value", ps->GetDuration(p));
          break;
        case Property::Boolean:
          state.writer.Boolean("value", ps->GetBool(p));
          break;
        case Property::Integer:
          state.writer.Number("value", ps->GetInteger(p));
          break;
      }
      state.writer.EndObject();
    }
    state.writer.EndArray();  // properties
    state.writer.StartArray("connections");
    for (const std::pair<Controllable *, size_t> &c : effect.Connections()) {
      state.writer.StartObject();
      state.writer.Number("input-index", c.second);
      state.writer.Number("folder", state.folderIds[&c.first->Parent()]);
      state.writer.String("name", c.first->Name());
      state.writer.EndObject();
    }
    state.writer.EndArray();   // connections
    state.writer.EndObject();  // effect
    state.controllablesWritten.insert(&effect);
  }
}

void writeControllable(WriteState &state, const Controllable &controllable) {
  if (state.controllablesWritten.count(&controllable) == 0) {
    const FixtureControl *fixtureControl =
        dynamic_cast<const FixtureControl *>(&controllable);
    const Chase *chase = dynamic_cast<const Chase *>(&controllable);
    const TimeSequence *tSequence =
        dynamic_cast<const TimeSequence *>(&controllable);
    const PresetCollection *presetCollection =
        dynamic_cast<const PresetCollection *>(&controllable);
    const Effect *effect = dynamic_cast<const Effect *>(&controllable);

    if (fixtureControl)
      writeFixtureControl(state, *fixtureControl);
    else if (chase)
      writeChase(state, *chase);
    else if (tSequence)
      writeTimeSequence(state, *tSequence);
    else if (presetCollection)
      writePresetCollection(state, *presetCollection);
    else if (effect)
      writeEffect(state, *effect);
    else
      throw std::runtime_error("Unknown controllable");
  }
  state.controllablesWritten.insert(&controllable);
}

void writeKeySceneItem(WriteState &state, const KeySceneItem &item) {
  state.writer.String("type", "key");
  state.writer.String("level", ToString(item.Level()));
}

void writeControlSceneItem(WriteState &state, const ControlSceneItem &item) {
  state.writer.String("type", "control");
  state.writer.Number("start-value", item.StartValue().UInt());
  state.writer.Number("end-value", item.EndValue().UInt());
  state.writer.String("controllable-ref", item.Controllable().Name());
  state.writer.Number("folder", state.folderIds[&item.Controllable().Parent()]);
}

void writeSceneItem(WriteState &state, const SceneItem &item) {
  state.writer.StartObject();

  state.writer.Number("offset", item.OffsetInMS());
  state.writer.Number("duration", item.DurationInMS());

  const KeySceneItem *keyItem = dynamic_cast<const KeySceneItem *>(&item);
  const ControlSceneItem *controlItem =
      dynamic_cast<const ControlSceneItem *>(&item);

  if (keyItem != nullptr)
    writeKeySceneItem(state, *keyItem);
  else
    writeControlSceneItem(state, *controlItem);

  state.writer.EndObject();
}

void writeScene(WriteState &state, const Scene &scene) {
  state.writer.StartObject();

  writeFolderAttributes(state, scene);
  state.writer.String("audio-file", scene.AudioFile());

  state.writer.StartArray("items");
  const std::multimap<double, std::unique_ptr<SceneItem>> &items =
      scene.SceneItems();
  for (const std::pair<const double, std::unique_ptr<SceneItem>> &sceneItem :
       items) {
    writeSceneItem(state, *sceneItem.second);
  }
  state.writer.EndArray();  // items

  state.writer.EndObject();
}

void writeFaderState(WriteState &state, const gui::FaderSetupState &guiState) {
  state.writer.StartObject();
  state.writer.String("name", guiState.name);
  state.writer.Boolean("active", guiState.isActive);
  state.writer.Boolean("solo", guiState.isSolo);
  state.writer.Number("fade-in", guiState.fadeInSpeed);
  state.writer.Number("fade-out", guiState.fadeOutSpeed);
  state.writer.Number("width", guiState.width);
  state.writer.Number("height", guiState.height);
  state.writer.StartArray("faders");
  for (const gui::FaderState &fader : guiState.faders) {
    state.writer.StartObject();
    state.writer.Boolean("is-toggle", fader.IsToggleButton());
    if (fader.IsToggleButton())
      state.writer.Boolean("new-toggle-column", fader.NewToggleButtonColumn());
    if (fader.GetSourceValue() != nullptr) {
      state.writer.Number("input-index",
                          fader.GetSourceValue()->Preset().InputIndex());
      state.writer.Number(
          "folder",
          state.folderIds[&fader.GetSourceValue()->GetControllable().Parent()]);
      state.writer.String("name",
                          fader.GetSourceValue()->GetControllable().Name());
    }
    state.writer.EndObject();
  }
  state.writer.EndArray();  // faders
  state.writer.EndObject();
}

void writeGUIState(WriteState &state) {
  state.writer.StartArray("states");
  for (const std::unique_ptr<gui::FaderSetupState> &fState :
       state.guiState->FaderSetups())
    writeFaderState(state, *fState);
  state.writer.EndArray();  // states
}

void writeGlightShow(WriteState &state) {
  state.writer.StartObject();

  writeFolders(state);

  state.writer.StartObject("theatre");

  Theatre &theatre = state.management.GetTheatre();

  const std::vector<std::unique_ptr<FixtureType>> &fixtureTypes =
      theatre.FixtureTypes();

  state.writer.StartArray("fixture-types");
  for (const std::unique_ptr<FixtureType> &ft : fixtureTypes)
    writeFixtureType(state, *ft);
  state.writer.EndArray();  // fixture-types

  state.writer.StartArray("fixtures");
  const std::vector<std::unique_ptr<Fixture>> &fixtures = theatre.Fixtures();
  for (const std::unique_ptr<Fixture> &f : fixtures) writeFixture(state, *f);
  state.writer.EndArray();  // fixtures

  state.writer.EndObject();  // theatre

  state.writer.StartArray("controls");

  const std::vector<std::unique_ptr<Controllable>> &controllables =
      state.management.Controllables();
  for (const std::unique_ptr<Controllable> &c : controllables)
    writeControllable(state, *c);
  state.writer.EndArray();  // controls

  const std::vector<std::unique_ptr<SourceValue>> &sourceValues =
      state.management.SourceValues();

  state.writer.StartArray("source-values");
  for (const std::unique_ptr<SourceValue> &sv : sourceValues)
    writePresetValue(state, sv->Preset());
  state.writer.EndArray();  // source_values

  state.writer.StartArray("scenes");

  Show &show = state.management.GetShow();

  const std::vector<std::unique_ptr<Scene>> &scenes = show.Scenes();
  for (const std::unique_ptr<Scene> &scene : scenes) writeScene(state, *scene);

  state.writer.EndArray();  // scenes

  if (state.guiState != nullptr) {
    state.writer.StartObject("gui");
    writeGUIState(state);
    state.writer.EndObject();  // gui
  }

  state.writer.EndObject();  // root object
}

}  // namespace

void Write(std::ostream &stream, Management &management,
           gui::GUIState *guiState) {
  WriteState state(management);
  state.writer = json::JsonWriter(stream);
  state.guiState = guiState;

  writeGlightShow(state);
}

void Write(const std::string &filename, Management &management,
           gui::GUIState *guiState) {
  std::ofstream file(filename);
  Write(file, management, guiState);
}

}  // namespace glight::system
