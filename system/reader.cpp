#include "reader.h"

#include "jsonreader.h"
#include "timepattern.h"

#include "theatre/chase.h"
#include "theatre/controllable.h"
#include "theatre/effect.h"
#include "theatre/fixture.h"
#include "theatre/fixturecontrol.h"
#include "theatre/fixturefunction.h"
#include "theatre/fixturegroup.h"
#include "theatre/fixturemode.h"
#include "theatre/fixturetype.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/presetvalue.h"
#include "theatre/theatre.h"
#include "theatre/timesequence.h"

#include "theatre/properties/propertyset.h"

#include "theatre/scenes/blackoutsceneitem.h"
#include "theatre/scenes/scene.h"

#include "gui/state/guistate.h"

#include <fstream>
#include <list>
#include <memory>

namespace glight::system {
namespace {

using namespace glight::theatre;

using json::Array;
using json::Node;
using json::Object;
using json::String;

using system::ObservingPtr;

void ParseNameAttr(const Object &node, NamedObject &object) {
  object.SetName(static_cast<const String &>(node["name"]).value);
}

void ParseFolderAttr(const Object &node,
                     const ObservingPtr<FolderObject> &object,
                     Management &management, bool hasFolder) {
  if (hasFolder) {
    size_t parent = ToNum(node["parent"]).AsSize();
    if (parent >= management.Folders().size())
      throw std::runtime_error("Invalid parent " + std::to_string(parent) +
                               " specified in file");
    management.Folders()[parent]->Add(object);
  }
  ParseNameAttr(node, *object);
}

void ParseFolderAttr(const Object &node,
                     const ObservingPtr<FolderObject> &object,
                     Management &management) {
  ParseFolderAttr(node, object, management, true);
}

void ParseFolders(const Array &node, Management &management) {
  for (const Node &child : node) {
    const Object &sub_folder = ToObj(child);
    const std::string &name = ToStr(sub_folder["name"]);
    auto parent = sub_folder.children.find("parent");
    if (parent == sub_folder.children.end()) {
      management.RootFolder().SetName(name);
    } else {
      const size_t p = ToNum(*parent->second).AsSize();
      if (p >= management.Folders().size())
        throw std::runtime_error("Reference to folder index " +
                                 std::to_string(p) + ", only have " +
                                 std::to_string(management.Folders().size()));
      management.AddFolder(*management.Folders()[p], name);
    }
  }
}

void ParseColorRangeParameters(const json::Object &node,
                               ColorRangeParameters &parameters) {
  const json::Array &ranges = ToArr(node["ranges"]);
  for (json::Node &item : ranges) {
    const json::Object &obj = ToObj(item);
    unsigned input_min = ToNum(obj["input-min"]).AsUInt();
    unsigned input_max = ToNum(obj["input-max"]).AsUInt();
    if (dynamic_cast<const json::Null *>(&obj["color"])) {
      parameters.GetRanges().emplace_back(input_min, input_max,
                                          std::optional<Color>());
    } else {
      const json::Object &color = ToObj(obj["color"]);
      const unsigned char r = ToNum(color["red"]).AsUChar();
      const unsigned char g = ToNum(color["green"]).AsUChar();
      const unsigned char b = ToNum(color["blue"]).AsUChar();
      parameters.GetRanges().emplace_back(
          input_min, input_max, std::optional<Color>(std::in_place, r, g, b));
    }
  }
}

void ParseRotationParameters(const json::Object &node,
                             RotationSpeedParameters &parameters) {
  const json::Array &ranges = ToArr(node["ranges"]);
  for (json::Node &item : ranges) {
    const json::Object &obj = ToObj(item);
    const unsigned input_min = ToNum(obj["input-min"]).AsUInt();
    const unsigned input_max = ToNum(obj["input-max"]).AsUInt();
    const int speed_min = ToNum(obj["speed-min"]).AsInt();
    const int speed_max = ToNum(obj["speed-max"]).AsInt();
    parameters.GetRanges().emplace_back(input_min, input_max, speed_min,
                                        speed_max);
  }
}

void ParseFixtureModeFunctions(const json::Array &node,
                               FixtureMode &fixture_mode) {
  std::vector<FixtureModeFunction> functions;
  for (const json::Node &child : node) {
    const json::Object &obj = ToObj(child);
    const FunctionType ft = GetFunctionType(ToStr(obj["type"]));
    const size_t dmx_offset = ToNum(obj["dmx-offset"]).AsSize();
    system::OptionalNumber<size_t> fine_channel;
    if (obj.contains("fine-channel-offset")) {
      fine_channel = ToNum(obj["fine-channel-offset"]).AsSize();
    }
    const unsigned shape = ToNum(obj["shape"]).AsUInt();
    FixtureModeFunction &new_function =
        functions.emplace_back(ft, dmx_offset, fine_channel, shape);
    new_function.SetPower(OptionalUInt(obj, "power", 0));
    switch (ft) {
      case FunctionType::ColorMacro:
      case FunctionType::ColorWheel:
        ParseColorRangeParameters(ToObj(obj["parameters"]),
                                  new_function.GetColorRangeParameters());
        break;
      case FunctionType::RotationSpeed:
        ParseRotationParameters(ToObj(obj["parameters"]),
                                new_function.GetRotationParameters());
        break;
      default:
        break;
    }
  }
  fixture_mode.SetFunctions(std::move(functions));
}

void ParseFixtureTypes(const json::Array &node, Management &management) {
  for (const Node &child : node) {
    const Object &ft_node = ToObj(child);
    TrackablePtr<FixtureType> ft = MakeTrackable<FixtureType>();
    ft->SetShortName(ToStr(ft_node["short-name"]));
    const std::string &class_name = ToStr(ft_node["fixture-class"]);
    ft->SetFixtureClass(GetFixtureClass(class_name));
    if (ft_node.contains("beam-angle")) {
      const double angle = ToNum(ft_node["beam-angle"]).AsDouble();
      ft->SetMinBeamAngle(angle);
      ft->SetMaxBeamAngle(angle);
    } else {
      ft->SetMinBeamAngle(ToNum(ft_node["min-beam-angle"]).AsDouble());
      ft->SetMaxBeamAngle(ToNum(ft_node["max-beam-angle"]).AsDouble());
    }
    if (ft_node.contains("min-pan")) {
      ft->SetMinPan(ToNum(ft_node["min-pan"]).AsDouble());
      ft->SetMaxPan(ToNum(ft_node["max-pan"]).AsDouble());
      ft->SetMinTilt(ToNum(ft_node["min-tilt"]).AsDouble());
      ft->SetMaxTilt(ToNum(ft_node["max-tilt"]).AsDouble());
    }
    ft->SetBrightness(ToNum(ft_node["brightness"]).AsDouble());
    ft->SetMaxPower(OptionalUInt(ft_node, "max-power", 0));
    ft->SetIdlePower(OptionalUInt(ft_node, "idle-power", 0));
    if (management.RootFolder().GetChildIfExists(ft->Name())) {
      throw std::runtime_error("Error in file: fixture type listed twice");
    }
    if (ft_node.contains("functions")) {
      // Old format: no modes
      FixtureMode &new_mode = ft->AddMode();
      new_mode.SetName("Default");
      ParseFixtureModeFunctions(ToArr(ft_node["functions"]), new_mode);
    } else {
      const Array &modes_array = ToArr(ft_node["modes"]);
      for (const Node &mode_node : modes_array) {
        const Object &mode_object = ToObj(mode_node);
        FixtureMode &new_mode = ft->AddMode();
        new_mode.SetName(ToStr(mode_object["name"]));
        ParseFixtureModeFunctions(ToArr(mode_object["functions"]), new_mode);
      }
    }
    ObservingPtr<FixtureType> observer =
        management.GetTheatre().AddFixtureTypePtr(std::move(ft));
    ParseFolderAttr(ft_node, observer, management);
  }
}

DmxChannel ParseDmxChannel(const Object &node) {
  DmxChannel channel;
  channel.SetUniverse(OptionalUInt(node, "universe", 0));
  channel.SetChannel(ToNum(node["channel"]).AsUInt());
  return channel;
}

void ParseFixtureFunction(const Object &node, Fixture &parentFixture) {
  FixtureFunction &function = parentFixture.AddFunction();
  ParseNameAttr(node, function);
  DmxChannel main_channel = ParseDmxChannel(ToObj(node["dmx-channel"]));
  std::optional<DmxChannel> fine_channel;
  if (node.contains("dmx-fine-channel")) {
    fine_channel = ParseDmxChannel(ToObj(node["dmx-fine-channel"]));
  }
  function.SetChannel(main_channel, fine_channel);
}

void ParseFixtures(const json::Array &node, Theatre &theatre) {
  for (const Node &child : node) {
    const Object &f_node = ToObj(child);
    FixtureType &type = *theatre.GetFixtureType(ToStr(f_node["type"]));
    const size_t mode_index = OptionalUInt(f_node, "mode-index", 0);
    if (mode_index >= type.Modes().size())
      throw std::runtime_error("Invalid mode index (" +
                               std::to_string(mode_index) +
                               ") in fixture of type " + type.Name());
    Fixture &fixture = *theatre.AddFixture(type.Modes()[mode_index]);
    ParseNameAttr(f_node, fixture);
    fixture.GetPosition().X() = ToNum(f_node["position-x"]).AsDouble();
    fixture.GetPosition().Y() = ToNum(f_node["position-y"]).AsDouble();
    fixture.GetPosition().Z() =
        OptionalDouble(f_node, "position-z", Fixture::kDefaultHeight);
    fixture.SetDirection(ToNum(f_node["direction"]).AsDouble());
    fixture.SetStaticTilt(
        OptionalDouble(f_node, "tilt", Fixture::kDefaultTilt));
    fixture.SetUpsideDown(OptionalBool(f_node, "upside-down", false));
    fixture.SetElectricPhase(OptionalSize(f_node, "electric-phase", 0));
    fixture.SetSymbol(FixtureSymbol(ToStr(f_node["symbol"])));
    fixture.ClearFunctions();

    const Array &functions = ToArr(f_node["functions"]);
    for (const Node &f : functions) {
      ParseFixtureFunction(ToObj(f), fixture);
    }
    if (fixture.Functions().size() != fixture.Mode().Functions().size()) {
      throw std::runtime_error(
          "Corrupted fixture found: " + fixture.Name() + " of type " +
          type.Name() + " has " + std::to_string(fixture.Functions().size()) +
          " functions, but should have " +
          std::to_string(fixture.Mode().Functions().size()) +
          " functions according to its type");
    }
  }
}

void ParseFixtureGroups(const json::Array &node, Management &management) {
  for (const Node &child : node) {
    const Object &f_node = ToObj(child);
    ObservingPtr<FixtureGroup> group =
        management.AddFixtureGroup().GetObserver();
    ParseFolderAttr(f_node, group, management);

    const Array &fixtures = ToArr(f_node["fixtures"]);
    for (const Node &f : fixtures) {
      ObservingPtr<Fixture> fixture =
          management.GetTheatre().GetFixturePtr(ToStr(f));
      group->Insert(std::move(fixture));
    }
  }
}

void ParseTheatre(const Object &node, Management &management) {
  Theatre &theatre = management.GetTheatre();
  theatre.SetWidth(OptionalDouble(node, "width", 10.0));
  theatre.SetDepth(OptionalDouble(node, "depth", 10.0));
  theatre.SetHeight(OptionalDouble(node, "height", 10.0));
  theatre.SetFixtureSymbolSize(
      OptionalDouble(node, "fixture-symbol-size", 0.5));

  ParseFixtureTypes(ToArr(node["fixture-types"]), management);
  ParseFixtures(ToArr(node["fixtures"]), management.GetTheatre());
}

void ParseFixtureControl(const Object &node, Management &management) {
  Fixture &fixture =
      management.GetTheatre().GetFixture(ToStr(node["fixture-ref"]));
  ObservingPtr<FixtureControl> control_ptr =
      management.AddFixtureControlPtr(fixture);
  ParseFolderAttr(node, control_ptr, management);
  FixtureControl &control = *control_ptr;
  if (node.contains("filters")) {
    const glight::json::Array &filters = ToArr(node["filters"]);
    for (const Node &node : filters) {
      const Object &filter = ToObj(node);
      const FilterType type = GetFilterType(ToStr(filter["type"]));
      control.AddFilter(Filter::Make(type));
    }
  }
}

void ParsePresetCollection(const Object &node, Management &management) {
  ObservingPtr<PresetCollection> collection_ptr =
      management.AddPresetCollectionPtr();
  ParseFolderAttr(node, collection_ptr, management);
  PresetCollection &collection = *collection_ptr;
  const Array &values = ToArr(node["values"]);
  for (const Node &value : values) {
    const Object &v = ToObj(value);
    Folder &folder = *management.Folders()[OptionalSize(v, "folder", 0)];
    FolderObject &obj = folder.GetChild(ToStr(v["controllable-ref"]));
    const size_t inputIndex = OptionalSize(v, "input-index", 0);
    Controllable *controllable = dynamic_cast<Controllable *>(&obj);
    if (controllable == nullptr)
      throw std::runtime_error(
          "Expecting a controllable in "
          "controllable-ref, but object named " +
          obj.Name() + " in folder " + folder.Name() +
          " is something different");
    PresetValue &pv = collection.AddPresetValue(*controllable, inputIndex);
    pv.SetValue(ControlValue(ToNum(v["value"]).AsUInt()));
  }
}

void ParseSequence(const Object &node, Sequence &sequence,
                   Management &management) {
  const Array &inputs = ToArr(node["inputs"]);
  for (Node &item_node : inputs) {
    const Object &item = ToObj(item_node);
    size_t input = OptionalSize(item, "input-index", 0);
    size_t folderId = OptionalSize(item, "folder", 0);
    Controllable &c = dynamic_cast<Controllable &>(
        management.Folders()[folderId]->GetChild(ToStr(item["name"])));
    sequence.Add(c, input);
  }
}

void ParseTrigger(const Object &node, Trigger &trigger) {
  trigger.SetType(GetTriggerType(ToStr(node["type"])));
  trigger.SetDelayInMs(ToNum(node["delay-in-ms"]).AsDouble());
  trigger.SetDelayInBeats(ToNum(node["delay-in-beats"]).AsDouble());
  trigger.SetDelayInSyncs(ToNum(node["delay-in-syncs"]).AsDouble());
}

Transition ParseTransition(const Object &node) {
  Transition transition;
  transition.SetType(GetTransitionType(ToStr(node["type"])));
  transition.SetLengthInMs(ToNum(node["length-in-ms"]).AsDouble());
  return transition;
}

void ParseChase(const Object &node, Management &management) {
  ObservingPtr<Chase> chase_ptr = management.AddChasePtr();
  ParseFolderAttr(node, chase_ptr, management);
  Chase &chase = *chase_ptr;
  ParseTrigger(ToObj(node["trigger"]), chase.GetTrigger());
  chase.GetTransition() = ParseTransition(ToObj(node["transition"]));
  ParseSequence(ToObj(node["sequence"]), chase.GetSequence(), management);
}

void ParseTimeSequence(const Object &node, Management &management) {
  ObservingPtr<TimeSequence> time_sequence_ptr =
      management.AddTimeSequencePtr();
  ParseFolderAttr(node, time_sequence_ptr, management);
  TimeSequence &time_sequence = *time_sequence_ptr;
  time_sequence.SetSustain(ToBool(node["sustain"]));
  time_sequence.SetRepeatCount(ToNum(node["repeat-count"]).AsSize());
  ParseSequence(ToObj(node["sequence"]), time_sequence.Sequence(), management);
  const Array &steps = ToArr(node["steps"]);
  size_t stepIndex = 0;
  for (Node &item : steps) {
    const Object &step_obj = ToObj(item);
    TimeSequence::Step &step = time_sequence.Steps().emplace_back();
    ParseTrigger(ToObj(step_obj["trigger"]), step.trigger);
    step.transition = ParseTransition(ToObj(step_obj["transition"]));
    ++stepIndex;
  }
  if (time_sequence.Steps().size() != time_sequence.Sequence().Size())
    throw std::runtime_error(
        "nr of steps in time sequence doesn't match sequence size");
}

void ParseEffect(const Object &node, Management &management) {
  EffectType type = NameToEffectType(ToStr(node["effect_type"]));
  const TrackablePtr<Controllable> &effect_ptr =
      management.AddEffect(Effect::Make(type));
  ParseFolderAttr(node, StaticObserverCast<Effect>(effect_ptr.GetObserver()),
                  management);
  std::unique_ptr<PropertySet> ps = PropertySet::Make(*effect_ptr);
  const Array &properties = ToArr(node["properties"]);
  for (const Node &item : properties) {
    const Object &p_node = ToObj(item);
    const std::string &propName = ToStr(p_node["name"]);
    Property &p = ps->GetProperty(propName);
    switch (p.GetType()) {
      case PropertyType::Choice:
        ps->SetChoice(p, ToStr(p_node["value"]));
        break;
      case PropertyType::ControlValue:
        ps->SetControlValue(p, ToNum(p_node["value"]).AsUInt());
        break;
      case PropertyType::Duration:
        ps->SetDuration(p, ToNum(p_node["value"]).AsDouble());
        break;
      case PropertyType::Boolean:
        ps->SetBool(p, ToBool(p_node["value"]));
        break;
      case PropertyType::Integer:
        ps->SetInteger(p, ToNum(p_node["value"]).AsInt());
        break;
      case PropertyType::TimePattern:
        ps->SetTimePattern(p, system::TimePattern(ToStr(p_node["value"])));
        break;
      case PropertyType::Transition:
        ps->SetTransition(p, ParseTransition(ToObj(p_node["value"])));
        break;
    }
  }
  const Array &connections = ToArr(node["connections"]);
  Effect &effect = static_cast<Effect &>(*effect_ptr);
  for (const Node &item : connections) {
    const Object &c_node = ToObj(item);
    const std::string &cName = ToStr(c_node["name"]);
    const size_t cInputIndex = OptionalSize(c_node, "input-index", 0);
    const size_t folderId = OptionalSize(c_node, "folder", 0);
    Folder *folder = management.Folders()[folderId].Get();
    effect.AddConnection(dynamic_cast<Controllable &>(folder->GetChild(cName)),
                         cInputIndex);
  }
}

KeySceneItem &ParseKeySceneItem(const Object &node, Scene &scene) {
  KeySceneItem *item = scene.AddKeySceneItem(ToNum(node["offset"]).AsDouble());
  item->SetLevel(GetKeySceneLevel(ToStr(node["level"])));
  return *item;
}

ControlSceneItem &ParseControlSceneItem(const Object &node, Scene &scene,
                                        Management &management) {
  const size_t folderId = OptionalSize(node, "folder", 0);
  Folder *folder = management.Folders()[folderId].Get();
  Controllable &controllable = static_cast<Controllable &>(
      folder->GetChild(ToStr(node["controllable-ref"])));
  ControlSceneItem *item = scene.AddControlSceneItem(
      ToNum(node["offset"]).AsDouble(), controllable, 0);
  item->StartValue().Set(ToNum(node["start-value"]).AsUInt());
  item->EndValue().Set(ToNum(node["end-value"]).AsUInt());
  return *item;
}

BlackoutSceneItem &ParseBlackoutSceneItem(const Object &node, Scene &scene) {
  BlackoutSceneItem &item =
      scene.AddBlackoutItem(ToNum(node["offset"]).AsDouble());
  item.SetOperation(GetBlackoutOperation(ToStr(node["operation"])));
  item.SetFadeSpeed(ToNum(node["fade-speed"]).AsDouble());
  return item;
}

void ParseSceneItem(const Object &node, Scene &scene, Management &management) {
  const std::string &type = ToStr(node["type"]);
  SceneItem *item;
  if (type == "key")
    item = &ParseKeySceneItem(node, scene);
  else if (type == "control")
    item = &ParseControlSceneItem(node, scene, management);
  else if (type == "blackout")
    item = &ParseBlackoutSceneItem(node, scene);
  else
    throw std::runtime_error(std::string("Invalid type for scene item: ") +
                             type);
  item->SetDurationInMS(ToNum(node["duration"]).AsDouble());
}

void ParseScene(const Object &scene_node, Management &management) {
  const ObservingPtr<Scene> &scene_ptr = management.AddScenePtr(false);
  ParseFolderAttr(scene_node, scene_ptr, management);
  Scene &scene = *scene_ptr;
  scene.SetAudioFile(ToStr(scene_node["audio-file"]));
  const Array &items = ToArr(scene_node["items"]);
  for (const Node &item_node : items) {
    ParseSceneItem(ToObj(item_node), scene, management);
  }
}

void ParseControls(const Array &node, Management &management) {
  for (const Node &control : node) {
    const std::string &t = ToStr(ToObj(control)["type"]);
    if (t == "fixture-control")
      ParseFixtureControl(ToObj(control), management);
    else if (t == "preset-collection")
      ParsePresetCollection(ToObj(control), management);
    else if (t == "chase")
      ParseChase(ToObj(control), management);
    else if (t == "time-sequence")
      ParseTimeSequence(ToObj(control), management);
    else if (t == "effect")
      ParseEffect(ToObj(control), management);
    else if (t == "scene")
      ParseScene(ToObj(control), management);
  }
}

SingleSourceValue ParseSingleSourceValue(const Object &object) {
  SingleSourceValue result;
  result.SetValue(ControlValue(OptionalUInt(object, "value", 0)));
  result.SetTargetValue(OptionalUInt(object, "target-value", 0));
  result.SetFadeSpeed(OptionalUInt(object, "fade-speed", 0.0));
  return result;
}

void ParseSourceValues(const Array &node, Management &management) {
  for (const Node &element : node) {
    const Object &object = ToObj(element);
    size_t folderId = OptionalSize(object, "folder", 0);
    const std::string name = ToStr(object["controllable-ref"]);
    Folder *folder = management.Folders()[folderId].Get();
    Controllable &controllable =
        dynamic_cast<Controllable &>(folder->GetChild(name));
    const size_t inputIndex = OptionalSize(object, "input-index", 0);
    SourceValue &value = management.AddSourceValue(controllable, inputIndex);
    value.A() = ParseSingleSourceValue(ToObj(object["a"]));
    value.B() = ParseSingleSourceValue(ToObj(object["b"]));
  }
}

void ParseGuiPresetRef(const Object &node, gui::FaderSetState &fader,
                       Management &management) {
  if (node.contains("name")) {
    // Old way of storing inputs ; support to be removed at a later time
    const size_t input = OptionalSize(node, "input-index", 0);
    const size_t folder_id = OptionalSize(node, "folder", 0);
    const std::string name = ToStr(node["name"]);
    Folder *folder = management.Folders()[folder_id].Get();
    Controllable &controllable =
        static_cast<Controllable &>(folder->GetChild(name));
    SourceValue *source = management.GetSourceValue(controllable, input);
    fader.faders.emplace_back(std::make_unique<gui::FaderState>(
        std::vector<theatre::SourceValue *>{source}));
  } else if (node.contains("source-values")) {
    std::vector<SourceValue *> sources;
    const Array &source_value_node = ToArr(node["source-values"]);
    for (const Node &source_value_item : source_value_node) {
      const Object &object = ToObj(source_value_item);
      if (object.contains("name")) {
        const size_t input = OptionalSize(object, "input-index", 0);
        const size_t folder_id = OptionalSize(object, "folder", 0);
        const std::string name = ToStr(object["name"]);
        Folder *folder = management.Folders()[folder_id].Get();
        Controllable &controllable =
            static_cast<Controllable &>(folder->GetChild(name));
        sources.emplace_back(management.GetSourceValue(controllable, input));
      } else {
        sources.emplace_back(nullptr);
      }
    }
    fader.faders.emplace_back(
        std::make_unique<gui::FaderState>(std::move(sources)));
  } else {
    fader.faders.emplace_back(std::make_unique<gui::FaderState>());
  }

  std::unique_ptr<gui::FaderState> &state = fader.faders.back();
  if (node.contains("is-toggle")) {
    // This is for older files and can be removed in the future
    state->SetFaderType(ToBool(node["is-toggle"])
                            ? gui::FaderControlType::ToggleButton
                            : gui::FaderControlType::Fader);
  } else {
    state->SetFaderType(gui::GetFaderControlType(ToStr(node["fader-type"])));
  }
  if (!IsFullColumnType(state->GetFaderType()))
    state->SetColumn(ToBool(node["new-toggle-column"]));
  state->SetDisplayName(
      OptionalBool(node, "display-name", state->DisplayName()));
  state->SetDisplayFlashButton(
      OptionalBool(node, "display-flash-button", state->DisplayFlashButton()));
  state->SetDisplayCheckButton(
      OptionalBool(node, "display-check-button", state->DisplayCheckButton()));
  state->SetOverlayFadeButtons(
      OptionalBool(node, "overlay-fade-buttons", state->OverlayFadeButtons()));
  state->SetLabel(OptionalString(node, "label", state->Label()));
}

void ParseGuiFaderSet(const Object &node, gui::GUIState &guiState,
                      Management &management) {
  guiState.FaderSets().emplace_back(std::make_unique<gui::FaderSetState>());
  gui::FaderSetState &fader_set = *guiState.FaderSets().back();
  fader_set.name = ToStr(node["name"]);
  fader_set.isActive = ToBool(node["active"]);
  fader_set.isSolo = ToBool(node["solo"]);
  fader_set.mode = gui::ToFaderSetMode(ToStr(node["mode"]));
  fader_set.fadeInSpeed = ToNum(node["fade-in"]).AsSize();
  fader_set.fadeOutSpeed = ToNum(node["fade-out"]).AsSize();
  fader_set.width = ToNum(node["width"]).AsSize();
  fader_set.height = ToNum(node["height"]).AsSize();
  // The position is no longer stored as this is not possible in gtk 4
  /*if (node.contains("position-x")) {
    fader_set.position_x = ToNum(node["position-x"]).AsInt();
    fader_set.position_y = ToNum(node["position-y"]).AsInt();
  }*/
  const Array &faders = ToArr(node["faders"]);
  for (const Node &item : faders) {
    const Object &fader_node = ToObj(item);
    ParseGuiPresetRef(fader_node, fader_set, management);
  }
}

void ParseGui(const Object &node, gui::GUIState &guiState,
              Management &management) {
  guiState.SetLayoutLocked(OptionalBool(node, "layout-locked", false));
  guiState.SetShowFixtures(OptionalBool(node, "show-fixtures", true));
  guiState.SetShowBeams(OptionalBool(node, "show-beams", true));
  guiState.SetShowProjections(OptionalBool(node, "show-projections", true));
  guiState.SetShowCrosshairs(OptionalBool(node, "show-crosshairs", true));
  guiState.SetShowStageBorders(OptionalBool(node, "show-stage-borders", true));

  if (node.contains("window-position-x")) {
    // window position not available since gtkmm 4
    // const int x = ToNum(node["window-position-x"]).AsInt();
    // const int y = ToNum(node["window-position-y"]).AsInt();
    const size_t width = ToNum(node["window-width"]).AsSize();
    const size_t height = ToNum(node["window-height"]).AsSize();
    guiState.SetWindowDimensions(width, height);
  }
  const Array &states = ToArr(node["states"]);
  for (const Node &item : states) {
    const Object &state_node = ToObj(item);
    ParseGuiFaderSet(state_node, guiState, management);
  }
}

void parseGlightShow(const Object &node, Management &management,
                     gui::GUIState *guiState) {
  ParseFolders(ToArr(node["folders"]), management);
  ParseTheatre(ToObj(node["theatre"]), management);
  ParseFixtureGroups(ToArr(node["fixture-groups"]), management);
  ParseControls(ToArr(node["controls"]), management);
  ParseSourceValues(ToArr(node["source-values"]), management);
  if (guiState != nullptr) {
    const auto &gui = node.children.find("gui");
    if (gui != node.children.end())
      ParseGui(ToObj(*gui->second), *guiState, management);
  }
}

}  // namespace

void Read(std::istream &stream, Management &management,
          gui::GUIState *guiState) {
  std::unique_ptr<Node> root = json::Parse(stream);
  parseGlightShow(ToObj(*root), management, guiState);
}

void Read(const std::string &filename, Management &management,
          gui::GUIState *guiState) {
  std::ifstream stream(filename);
  if (!stream) throw std::runtime_error("Failed to open file");
  Read(stream, management, guiState);
}

void ImportFixtureTypes(const std::string &filename,
                        theatre::Management &management) {
  theatre::Management file_management(management.Settings());
  system::Read(filename, file_management);
  const std::vector<TrackablePtr<FixtureType>> &new_types =
      file_management.GetTheatre().FixtureTypes();
  for (const TrackablePtr<FixtureType> &type : new_types) {
    if (!management.RootFolder().GetChildIfExists(type->Name())) {
      const TrackablePtr<FixtureType> &added_type =
          management.GetTheatre().AddFixtureType(
              MakeTrackable<FixtureType>(*type));
      management.RootFolder().Add(added_type.GetObserver());
    }
  }
}

}  // namespace glight::system
