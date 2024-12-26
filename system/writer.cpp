#include <fstream>
#include <queue>

#include "writer.h"

#include "theatre/chase.h"
#include "theatre/controllable.h"
#include "theatre/effect.h"
#include "theatre/fixture.h"
#include "theatre/fixturecontrol.h"
#include "theatre/fixturefunction.h"
#include "theatre/fixturegroup.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/presetvalue.h"
#include "theatre/theatre.h"
#include "theatre/timesequence.h"

#include "theatre/properties/propertyset.h"

#include "theatre/scenes/blackoutsceneitem.h"
#include "theatre/scenes/controlsceneitem.h"
#include "theatre/scenes/keysceneitem.h"
#include "theatre/scenes/scene.h"

#include "gui/state/guistate.h"

namespace glight::system {
namespace {

using namespace glight::theatre;

struct WriteState {
  explicit WriteState(Management &management_) : management(management_) {}
  json::JsonWriter writer;
  std::set<const Controllable *> controllablesWritten;
  std::map<const Folder *, size_t> folderIds;
  Management &management;
  gui::GUIState *guiState = nullptr;
};

void writeControllable(WriteState &state, const Controllable &controllable);

void writeFolders(WriteState &state) {
  state.writer.StartArray("folders");
  std::queue<const Folder *> folders;
  for (const std::unique_ptr<Folder> &folder : state.management.Folders())
    folders.emplace(folder.get());
  size_t next_folder_id = 0;
  while (!folders.empty()) {
    const Folder &folder = *folders.front();
    folders.pop();
    std::map<const Folder *, size_t>::iterator iter;
    if (!folder.IsRoot()) {
      iter = state.folderIds.find(&folder.Parent());
    }
    if (folder.IsRoot() || iter != state.folderIds.end()) {
      state.folderIds.emplace(&folder, next_folder_id);
      state.writer.StartObject();
      state.writer.Number("id", next_folder_id);
      ++next_folder_id;
      state.writer.String("name", folder.Name());
      if (!folder.IsRoot()) {
        state.writer.Number("parent", iter->second);
      }
      state.writer.EndObject();
    } else {
      // Put folder at back of queue: the parent has not been written
      // yet.
      folders.emplace(&folder);
    }
  }
  state.writer.EndArray();
}

void writeNameAttributes(WriteState &state, const NamedObject &obj) {
  state.writer.String("name", obj.Name());
}

void writeFolderAttributes(WriteState &state, const FolderObject &obj) {
  writeNameAttributes(state, obj);
  if (obj.IsRoot())
    throw std::runtime_error("Folder object '" + obj.Name() +
                             "' has no parent");
  state.writer.Number("parent", state.folderIds.find(&obj.Parent())->second);
}

void writeDmxChannel(WriteState &state, const DmxChannel &dmxChannel,
                     const char *name) {
  state.writer.StartObject(name);
  state.writer.Number("universe", dmxChannel.Universe());
  state.writer.Number("channel", dmxChannel.Channel());
  state.writer.EndObject();
}

void writeFixtureFunction(WriteState &state,
                          const FixtureFunction &fixtureFunction) {
  state.writer.StartObject();
  state.writer.String("name", fixtureFunction.Name());
  writeDmxChannel(state, fixtureFunction.MainChannel(), "dmx-channel");
  if (fixtureFunction.FineChannel()) {
    writeDmxChannel(state, *fixtureFunction.FineChannel(), "dmx-fine-channel");
  }
  state.writer.EndObject();
}

void writeFixture(WriteState &state, const Fixture &fixture) {
  state.writer.StartObject();
  writeNameAttributes(state, fixture);
  state.writer.String("type", fixture.Type().Name());
  state.writer.Number("position-x", fixture.GetPosition().X());
  state.writer.Number("position-y", fixture.GetPosition().Y());
  state.writer.Number("direction", fixture.Direction());
  if (fixture.Tilt() != 0.0) state.writer.Number("tilt", fixture.Tilt());
  if (fixture.IsUpsideDown())
    state.writer.Boolean("upside-down", fixture.IsUpsideDown());
  if (fixture.ElectricPhase() != 0)
    state.writer.Number("electric-phase", fixture.ElectricPhase());
  state.writer.String("symbol", fixture.Symbol().Name());
  const std::vector<std::unique_ptr<FixtureFunction>> &functions =
      fixture.Functions();
  state.writer.StartArray("functions");
  for (const std::unique_ptr<FixtureFunction> &ff : functions)
    writeFixtureFunction(state, *ff);
  state.writer.EndArray();   // functions
  state.writer.EndObject();  // fixture
}

void writeFixtureGroup(WriteState &state, const FixtureGroup &group) {
  state.writer.StartObject();
  writeFolderAttributes(state, group);
  const std::vector<Fixture *> fixtures = group.Fixtures();
  state.writer.StartArray("fixtures");
  for (const Fixture *fixture : fixtures) {
    // TODO fixture should become a FolderObject and this should be the full
    // path
    state.writer.String(fixture->Name());
  }
  state.writer.EndArray();   // fixtures
  state.writer.EndObject();  // fixture-group
}

void witeMacroParameters(WriteState &state, const ColorRangeParameters &pars) {
  state.writer.StartObject("parameters");
  state.writer.StartArray("ranges");
  for (const ColorRangeParameters::Range &range : pars.GetRanges()) {
    state.writer.StartObject();
    state.writer.Number("input-min", range.input_min);
    state.writer.Number("input-max", range.input_max);
    if (range.color) {
      state.writer.StartObject("color");
      state.writer.Number("red", range.color->Red());
      state.writer.Number("green", range.color->Green());
      state.writer.Number("blue", range.color->Blue());
      state.writer.EndObject();  // color
    } else {
      state.writer.Null("color");
    }
    state.writer.EndObject();  // range
  }
  state.writer.EndArray();
  state.writer.EndObject();  // parameters
}

void writeRotationParameters(WriteState &state,
                             const RotationSpeedParameters &pars) {
  state.writer.StartObject("parameters");
  state.writer.StartArray("ranges");
  for (const RotationSpeedParameters::Range &range : pars.GetRanges()) {
    state.writer.StartObject();
    state.writer.Number("input-min", range.input_min);
    state.writer.Number("input-max", range.input_max);
    state.writer.Number("speed-min", range.speed_min);
    state.writer.Number("speed-max", range.speed_max);
    state.writer.EndObject();  // range
  }
  state.writer.EndArray();
  state.writer.EndObject();  // parameters
}

void writeFixtureTypeFunction(WriteState &state,
                              const FixtureTypeFunction &function) {
  state.writer.StartObject();
  state.writer.String("type", ToString(function.Type()));
  state.writer.Number("dmx-offset", function.DmxOffset());
  if (function.FineChannelOffset())
    state.writer.Number("fine-channel-offset", *function.FineChannelOffset());
  state.writer.Number("shape", function.Shape());
  if (function.Power() != 0.0) state.writer.Number("power", function.Power());
  switch (function.Type()) {
    case FunctionType::ColorMacro:
    case FunctionType::ColorWheel:
      witeMacroParameters(state, function.GetColorRangeParameters());
      break;
    case FunctionType::RotationSpeed:
      writeRotationParameters(state, function.GetRotationSpeedParameters());
      break;
    default:
      break;
  }
  state.writer.EndObject();
}

void writeFixtureType(WriteState &state, const FixtureType &fixtureType) {
  state.writer.StartObject();
  writeFolderAttributes(state, fixtureType);
  state.writer.String("short-name", fixtureType.ShortName());
  state.writer.String("fixture-class",
                      FixtureType::ClassName(fixtureType.GetFixtureClass()));
  state.writer.Number("shape-count", fixtureType.ShapeCount());
  state.writer.Number("min-beam-angle", fixtureType.MinBeamAngle());
  state.writer.Number("max-beam-angle", fixtureType.MaxBeamAngle());
  state.writer.Number("min-pan", fixtureType.MinPan());
  state.writer.Number("max-pan", fixtureType.MaxPan());
  state.writer.Number("min-tilt", fixtureType.MinTilt());
  state.writer.Number("max-tilt", fixtureType.MaxTilt());
  state.writer.Number("brightness", fixtureType.Brightness());
  if (fixtureType.MaxPower() != 0.0)
    state.writer.Number("max-power", fixtureType.MaxPower());
  if (fixtureType.IdlePower() != 0.0)
    state.writer.Number("idle-power", fixtureType.IdlePower());
  state.writer.StartArray("functions");
  for (const FixtureTypeFunction &f : fixtureType.Functions()) {
    writeFixtureTypeFunction(state, f);
  }
  state.writer.EndArray();  // functions
  state.writer.EndObject();
}

void writeSingleSourceValue(WriteState &state,
                            const SingleSourceValue &singleSourceValue) {
  state.writer.Number("value", singleSourceValue.Value().UInt());
  state.writer.Number("target-value", singleSourceValue.TargetValue());
  state.writer.Number("fade-speed", singleSourceValue.FadeSpeed());
}

void writeSourceValue(WriteState &state, const SourceValue &sourceValue) {
  writeControllable(state, sourceValue.GetControllable());

  state.writer.StartObject();
  state.writer.String("controllable-ref", sourceValue.GetControllable().Name());
  state.writer.Number("input-index", sourceValue.InputIndex());
  state.writer.Number("folder",
                      state.folderIds[&sourceValue.GetControllable().Parent()]);
  state.writer.StartObject("a");
  writeSingleSourceValue(state, sourceValue.A());
  state.writer.EndObject();
  state.writer.StartObject("b");
  writeSingleSourceValue(state, sourceValue.B());
  state.writer.EndObject();
  state.writer.EndObject();
}

void writePresetValue(WriteState &state, const PresetValue &presetValue) {
  writeControllable(state, presetValue.GetControllable());

  state.writer.StartObject();
  state.writer.String("controllable-ref", presetValue.GetControllable().Name());
  state.writer.Number("input-index", presetValue.InputIndex());
  state.writer.Number("folder",
                      state.folderIds[&presetValue.GetControllable().Parent()]);
  state.writer.Number("value", presetValue.Value().UInt());
  state.writer.EndObject();
}

void writePresetCollection(WriteState &state,
                           const class PresetCollection &presetCollection) {
  const std::vector<std::unique_ptr<PresetValue>> &values =
      presetCollection.PresetValues();
  for (const std::unique_ptr<PresetValue> &pv : values)
    writeControllable(state, pv->GetControllable());

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
  state.writer.String("fixture-ref", control.GetFixture().Name());
  if (!control.Filters().empty()) {
    state.writer.StartArray("filters");
    for (const std::unique_ptr<Filter> &filter : control.Filters()) {
      state.writer.StartObject();
      state.writer.String("type", ToString(filter->GetType()));
      state.writer.EndObject();
    }
    state.writer.EndArray();
  }
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

void writeTransition(WriteState &state, const Transition &transition,
                     const char *name = "transition") {
  state.writer.StartObject(name);
  state.writer.Number("length-in-ms", transition.LengthInMs());
  state.writer.String("type", ToString(transition.Type()));
  state.writer.EndObject();
}

void writeSequence(WriteState &state, const Sequence &sequence) {
  state.writer.StartObject("sequence");
  state.writer.StartArray("inputs");
  for (const Input &input : sequence.List()) {
    state.writer.StartObject();
    state.writer.Number("input-index", input.InputIndex());
    state.writer.Number("folder",
                        state.folderIds[&input.GetControllable()->Parent()]);
    state.writer.String("name", input.GetControllable()->Name());
    state.writer.EndObject();
  }
  state.writer.EndArray();
  state.writer.EndObject();
}

void writeChase(WriteState &state, const Chase &chase) {
  const std::vector<Input> &list = chase.GetSequence().List();
  for (const Input &input : list)
    writeControllable(state, *input.GetControllable());

  state.writer.StartObject();
  state.writer.String("type", "chase");
  writeFolderAttributes(state, chase);
  writeTrigger(state, chase.GetTrigger());
  writeTransition(state, chase.GetTransition());
  writeSequence(state, chase.GetSequence());
  state.writer.EndObject();
}

void writeTimeSequence(WriteState &state, const TimeSequence &timeSequence) {
  const std::vector<Input> &list = timeSequence.Sequence().List();
  for (const Input &input : list)
    writeControllable(state, *input.GetControllable());

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
  if (!state.controllablesWritten.contains(&effect)) {
    for (const std::pair<Controllable *, size_t> &c : effect.Connections())
      writeControllable(state, *c.first);

    state.writer.StartObject();
    state.writer.String("type", "effect");
    writeFolderAttributes(state, effect);
    state.writer.String("effect_type", EffectTypeToName(effect.GetType()));
    std::unique_ptr<PropertySet> ps = PropertySet::Make(effect);

    // the number and name of the effect controls are implied from the
    // effect type, so do not require to be stored.

    state.writer.StartArray("properties");
    for (const Property &p : *ps) {
      state.writer.StartObject();
      state.writer.String("name", p.Name());
      switch (p.GetType()) {
        case PropertyType::Choice:
          state.writer.String("value", ps->GetChoice(p));
          break;
        case PropertyType::ControlValue:
          state.writer.Number("value", ps->GetControlValue(p));
          break;
        case PropertyType::Duration:
          state.writer.Number("value", ps->GetDuration(p));
          break;
        case PropertyType::Boolean:
          state.writer.Boolean("value", ps->GetBool(p));
          break;
        case PropertyType::Integer:
          state.writer.Number("value", ps->GetInteger(p));
          break;
        case PropertyType::Transition:
          writeTransition(state, ps->GetTransition(p), "value");
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

void writeKeySceneItem(WriteState &state, const KeySceneItem &item) {
  state.writer.String("type", "key");
  state.writer.String("level", ToString(item.Level()));
}

void writeControlSceneItem(WriteState &state, const ControlSceneItem &item) {
  state.writer.String("type", "control");
  state.writer.Number("start-value", item.StartValue().UInt());
  state.writer.Number("end-value", item.EndValue().UInt());
  state.writer.String("controllable-ref", item.GetControllable().Name());
  state.writer.Number("folder",
                      state.folderIds[&item.GetControllable().Parent()]);
}

void writeBlackoutSceneItem(WriteState &state, const BlackoutSceneItem &item) {
  state.writer.String("type", "blackout");
  state.writer.String("operation", ToString(item.Operation()));
  state.writer.Number("fade-speed", item.FadeSpeed());
}

void writeSceneItem(WriteState &state, const SceneItem &item) {
  state.writer.StartObject();

  state.writer.Number("offset", item.OffsetInMS());
  state.writer.Number("duration", item.DurationInMS());

  if (const KeySceneItem *keyItem = dynamic_cast<const KeySceneItem *>(&item);
      keyItem)
    writeKeySceneItem(state, *keyItem);
  else if (const ControlSceneItem *controlItem =
               dynamic_cast<const ControlSceneItem *>(&item);
           controlItem)
    writeControlSceneItem(state, *controlItem);
  else {
    const BlackoutSceneItem *blackout =
        dynamic_cast<const BlackoutSceneItem *>(&item);
    writeBlackoutSceneItem(state, *blackout);
  }

  state.writer.EndObject();
}

void writeScene(WriteState &state, const Scene &scene) {
  for (size_t i = 0; i != scene.NOutputs(); ++i) {
    writeControllable(state, *scene.Output(i).first);
  }

  state.writer.StartObject();

  state.writer.String("type", "scene");
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

void writeControllable(WriteState &state, const Controllable &controllable) {
  if (!state.controllablesWritten.contains(&controllable)) {
    if (const FixtureControl *fixtureControl =
            dynamic_cast<const FixtureControl *>(&controllable);
        fixtureControl)
      writeFixtureControl(state, *fixtureControl);
    else if (const Chase *chase = dynamic_cast<const Chase *>(&controllable);
             chase)
      writeChase(state, *chase);
    else if (const TimeSequence *tSequence =
                 dynamic_cast<const TimeSequence *>(&controllable);
             tSequence)
      writeTimeSequence(state, *tSequence);
    else if (const PresetCollection *presetCollection =
                 dynamic_cast<const PresetCollection *>(&controllable);
             presetCollection)
      writePresetCollection(state, *presetCollection);
    else if (const Effect *effect = dynamic_cast<const Effect *>(&controllable);
             effect)
      writeEffect(state, *effect);
    else if (const Scene *scene = dynamic_cast<const Scene *>(&controllable);
             scene)
      writeScene(state, *scene);
    else
      throw std::runtime_error("Unknown controllable");
  }
  state.controllablesWritten.insert(&controllable);
}

void writeFaderState(WriteState &state, const gui::FaderState &fader) {
  state.writer.StartObject();
  state.writer.String("fader-type", ToString(fader.GetFaderType()));
  if (!IsFullColumnType(fader.GetFaderType()))
    state.writer.Boolean("new-toggle-column", fader.NewToggleButtonColumn());
  state.writer.Boolean("display-name", fader.DisplayName());
  state.writer.Boolean("display-flash-button", fader.DisplayFlashButton());
  state.writer.Boolean("display-check-button", fader.DisplayCheckButton());
  state.writer.Boolean("overlay-fade-buttons", fader.OverlayFadeButtons());
  if (!fader.Label().empty()) state.writer.String("label", fader.Label());
  const std::vector<SourceValue *> &sources = fader.GetSourceValues();
  state.writer.StartArray("source-values");
  for (SourceValue *source : sources) {
    state.writer.StartObject();
    if (source != nullptr) {
      state.writer.Number("folder",
                          state.folderIds[&source->GetControllable().Parent()]);
      state.writer.String("name", source->GetControllable().Name());
      state.writer.Number("input-index", source->InputIndex());
    }
    state.writer.EndObject();
  }
  state.writer.EndArray();
  state.writer.EndObject();
}

void writeFaderSetState(WriteState &state, const gui::FaderSetState &guiState) {
  state.writer.StartObject();
  state.writer.String("name", guiState.name);
  state.writer.Boolean("active", guiState.isActive);
  state.writer.Boolean("solo", guiState.isSolo);
  state.writer.String("mode", ToString(guiState.mode));
  state.writer.Number("fade-in", guiState.fadeInSpeed);
  state.writer.Number("fade-out", guiState.fadeOutSpeed);
  state.writer.Number("width", guiState.width);
  state.writer.Number("height", guiState.height);
  state.writer.Number("position-x", guiState.position_x);
  state.writer.Number("position-y", guiState.position_y);
  state.writer.StartArray("faders");
  for (const std::unique_ptr<gui::FaderState> &fader : guiState.faders) {
    writeFaderState(state, *fader);
  }
  state.writer.EndArray();  // faders
  state.writer.EndObject();
}

void writeGUIState(WriteState &state) {
  if (state.guiState->LayoutLocked()) {
    state.writer.Boolean("layout-locked", true);
  }
  state.writer.Number("window-position-x", state.guiState->WindowPositionX());
  state.writer.Number("window-position-y", state.guiState->WindowPositionY());
  state.writer.Number("window-width", state.guiState->WindowWidth());
  state.writer.Number("window-height", state.guiState->WindowHeight());

  state.writer.StartArray("states");
  for (const std::unique_ptr<gui::FaderSetState> &fState :
       state.guiState->FaderSets())
    writeFaderSetState(state, *fState);
  state.writer.EndArray();  // states
}

void writeGlightShow(WriteState &state) {
  state.writer.StartObject();

  writeFolders(state);

  state.writer.StartObject("theatre");

  Theatre &theatre = state.management.GetTheatre();

  state.writer.Number("width", theatre.Width());
  state.writer.Number("depth", theatre.Depth());
  state.writer.Number("height", theatre.Height());
  state.writer.Number("fixture-symbol-size", theatre.FixtureSymbolSize());

  const std::vector<std::unique_ptr<FixtureType>> &fixtureTypes =
      theatre.FixtureTypes();

  state.writer.StartArray("fixture-types");
  for (const std::unique_ptr<FixtureType> &ft : fixtureTypes)
    writeFixtureType(state, *ft);
  state.writer.EndArray();  // fixture-types

  state.writer.StartArray("fixtures");
  const std::vector<system::TrackablePtr<Fixture>> &fixtures =
      theatre.Fixtures();
  for (const system::TrackablePtr<Fixture> &f : fixtures)
    writeFixture(state, *f);
  state.writer.EndArray();  // fixtures

  state.writer.EndObject();  // theatre

  state.writer.StartArray("fixture-groups");
  const std::vector<std::unique_ptr<FixtureGroup>> &groups =
      state.management.FixtureGroups();
  for (const std::unique_ptr<FixtureGroup> &f : groups)
    writeFixtureGroup(state, *f);
  state.writer.EndArray();  // fixture-groups

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
    writeSourceValue(state, *sv);
  state.writer.EndArray();  // source_values

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
