#include "reader.h"

#include "jsonreader.h"

#include "../theatre/properties/propertyset.h"

#include "../theatre/chase.h"
#include "../theatre/controllable.h"
#include "../theatre/effect.h"
#include "../theatre/fixture.h"
#include "../theatre/fixturecontrol.h"
#include "../theatre/fixturefunction.h"
#include "../theatre/folder.h"
#include "../theatre/presetvalue.h"
#include "../theatre/scene.h"
#include "../theatre/show.h"
#include "../theatre/theatre.h"
#include "../theatre/timesequence.h"

#include "../gui/guistate.h"

#include <fstream>

namespace glight::system {
namespace {

using namespace glight::theatre;

using json::Array;
using json::Boolean;
using json::Node;
using json::Number;
using json::Object;
using json::String;

void ParseNameAttr(const Object &node, NamedObject &object) {
  object.SetName(static_cast<const String &>(node["name"]).value);
}

void ParseFolderAttr(const Object &node, FolderObject &object,
                     Management &management, bool hasFolder = true) {
  if (hasFolder) {
    size_t parent = ToNum(node["parent"]).AsSize();
    if (parent >= management.Folders().size())
      throw std::runtime_error("Invalid parent specified in file");
    management.Folders()[parent]->Add(object);
  }
  ParseNameAttr(node, object);
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
      management.AddFolder(*management.Folders()[p], name);
    }
  }
}

void ParseMacroParameters(const json::Object &node,
                          MacroParameters &parameters) {
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
                             RotationParameters &parameters) {
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

void ParseFixtureTypeFunctions(const json::Array &node,
                               FixtureType &fixture_type) {
  std::vector<FixtureTypeFunction> functions;
  for (const json::Node &child : node) {
    const json::Object &obj = ToObj(child);
    const FunctionType ft = GetFunctionType(ToStr(obj["type"]));
    const size_t dmx_offset = ToNum(obj["dmx-offset"]).AsSize();
    const bool is_16_bit = ToBool(obj["is-16-bit"]);
    const unsigned shape = ToNum(obj["shape"]).AsUInt();
    FixtureTypeFunction &new_function =
        functions.emplace_back(ft, dmx_offset, is_16_bit, shape);
    switch (ft) {
      case FunctionType::ColorMacro:
        ParseMacroParameters(ToObj(obj["parameters"]),
                             new_function.GetMacroParameters());
        break;
      case FunctionType::Rotation:
        ParseRotationParameters(ToObj(obj["parameters"]),
                                new_function.GetRotationParameters());
        break;
      default:
        break;
    }
  }
  fixture_type.SetFunctions(std::move(functions));
}

void ParseFixtureTypes(const json::Array &node, Management &management) {
  for (const Node &child : node) {
    const Object &ft_node = ToObj(child);
    const std::string &class_name = ToStr(ft_node["fixture-class"]);
    FixtureType ft;
    ft.SetFixtureClass(FixtureType::NameToClass(class_name));
    FixtureType &new_type = management.GetTheatre().AddFixtureType(ft);
    if (management.RootFolder().GetChildIfExists(new_type.Name())) {
      throw std::runtime_error("Error in file: fixture type listed twice");
    }
    ParseFolderAttr(ft_node, new_type, management);
    ParseFixtureTypeFunctions(ToArr(ft_node["functions"]), new_type);
  }
}

void ParseDmxChannel(const Object &node, DmxChannel &dmxChannel) {
  dmxChannel.SetUniverse(ToNum(node["universe"]).AsInt());
  dmxChannel.SetChannel(ToNum(node["channel"]).AsInt());
  dmxChannel.SetDefaultMixStyle(GetMixStyle(ToStr(node["default-mix-style"])));
}

void ParseFixtureFunction(const Object &node, Fixture &parentFixture) {
  FixtureFunction &function =
      parentFixture.AddFunction(GetFunctionType(ToStr(node["type"])));
  ParseNameAttr(node, function);
  DmxChannel channel;
  ParseDmxChannel(ToObj(node["dmx-channel"]), channel);
  function.SetChannel(channel);
}

void ParseFixtures(const json::Array &node, Theatre &theatre) {
  for (const Node &child : node) {
    const Object &f_node = ToObj(child);
    FixtureType &type = theatre.GetFixtureType(ToStr(f_node["type"]));
    Fixture &fixture = theatre.AddFixture(type);
    ParseNameAttr(f_node, fixture);
    fixture.GetPosition().X() = ToNum(f_node["position-x"]).AsDouble();
    fixture.GetPosition().Y() = ToNum(f_node["position-y"]).AsDouble();
    fixture.SetSymbol(FixtureSymbol(ToStr(f_node["symbol"])));
    fixture.ClearFunctions();

    const Array &functions = ToArr(f_node["functions"]);
    for (const Node &f : functions) {
      ParseFixtureFunction(ToObj(f), fixture);
    }
  }
}

void ParseTheatre(const Object &node, Management &management) {
  ParseFixtureTypes(ToArr(node["fixture-types"]), management);
  ParseFixtures(ToArr(node["fixtures"]), management.GetTheatre());
}

void ParseFixtureControl(const Object &node, Management &management) {
  Fixture &fixture =
      management.GetTheatre().GetFixture(ToStr(node["fixture-ref"]));
  FixtureControl &control = management.AddFixtureControl(fixture);
  ParseFolderAttr(node, control, management);
}

void ParsePresetCollection(const Object &node, Management &management) {
  PresetCollection &collection = management.AddPresetCollection();
  ParseFolderAttr(node, collection, management);
  const Array &values = ToArr(node["values"]);
  for (const Node &value : values) {
    const Object &v = ToObj(value);
    Folder &folder = *management.Folders()[ToNum(v["folder"]).AsSize()];
    FolderObject &obj = folder.GetChild(ToStr(v["controllable-ref"]));
    const size_t inputIndex = ToNum(v["input-index"]).AsSize();
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
    size_t input = ToNum(item["input-index"]).AsSize();
    size_t folderId = ToNum(item["folder"]).AsSize();
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

void ParseTransition(const Object &node, Transition &transition) {
  transition.SetType(GetTransitionType(ToStr(node["type"])));
  transition.SetLengthInMs(ToNum(node["length-in-ms"]).AsDouble());
}

void ParseChase(const Object &node, Management &management) {
  Chase &chase = management.AddChase();
  ParseFolderAttr(node, chase, management);
  ParseTrigger(ToObj(node["trigger"]), chase.Trigger());
  ParseTransition(ToObj(node["transition"]), chase.Transition());
  ParseSequence(ToObj(node["sequence"]), chase.Sequence(), management);
}

void ParseTimeSequence(const Object &node, Management &management) {
  TimeSequence &timeSequence = management.AddTimeSequence();
  ParseFolderAttr(node, timeSequence, management);
  timeSequence.SetSustain(ToBool(node["sustain"]));
  timeSequence.SetRepeatCount(ToNum(node["repeat-count"]).AsSize());
  ParseSequence(ToObj(node["sequence"]), timeSequence.Sequence(), management);
  const Array &steps = ToArr(node["steps"]);
  size_t stepIndex = 0;
  for (Node &item : steps) {
    const Object &step_obj = ToObj(item);
    TimeSequence::Step &step = timeSequence.Steps().emplace_back();
    ParseTrigger(ToObj(step_obj["trigger"]), step.trigger);
    ParseTransition(ToObj(step_obj["transition"]), step.transition);
    ++stepIndex;
  }
  if (timeSequence.Steps().size() != timeSequence.Sequence().Size())
    throw std::runtime_error(
        "nr of steps in time sequence doesn't match sequence size");
}

void ParsePresetValue(const Object &node, Management &management) {
  size_t folderId = ToNum(node["folder"]).AsSize();
  const std::string name = ToStr(node["controllable-ref"]);
  Folder *folder = management.Folders()[folderId].get();
  Controllable &controllable =
      dynamic_cast<Controllable &>(folder->GetChild(name));
  const size_t inputIndex = ToNum(node["input-index"]).AsSize();
  SourceValue &value = management.AddSourceValue(controllable, inputIndex);
  value.Preset().SetValue(ControlValue(ToNum(node["value"]).AsUInt()));
}

void ParseEffect(const Object &node, Management &management) {
  Effect::Type type = Effect::NameToType(ToStr(node["effect_type"]));
  std::unique_ptr<Effect> effect = Effect::Make(type);
  ParseFolderAttr(node, *effect, management);
  Effect *effectPtr = &management.AddEffect(std::move(effect));
  std::unique_ptr<PropertySet> ps = PropertySet::Make(*effectPtr);
  const Array &properties = ToArr(node["properties"]);
  for (const Node &item : properties) {
    const Object &p_node = ToObj(item);
    const std::string &propName = ToStr(p_node["name"]);
    Property &p = ps->GetProperty(propName);
    switch (p.GetType()) {
      case Property::Choice:
        ps->SetChoice(p, ToStr(p_node["value"]));
        break;
      case Property::ControlValue:
        ps->SetControlValue(p, ToNum(p_node["value"]).AsUInt());
        break;
      case Property::Duration:
        ps->SetDuration(p, ToNum(p_node["value"]).AsDouble());
        break;
      case Property::Boolean:
        ps->SetBool(p, ToBool(p_node["value"]));
        break;
      case Property::Integer:
        ps->SetInteger(p, ToNum(p_node["value"]).AsInt());
        break;
    }
  }
  const Array &connections = ToArr(node["connections"]);
  for (const Node &item : connections) {
    const Object &c_node = ToObj(item);
    const std::string &cName = ToStr(c_node["name"]);
    const size_t cInputIndex = ToNum(c_node["input-index"]).AsSize();
    const size_t folderId = ToNum(c_node["folder"]).AsSize();
    Folder *folder = management.Folders()[folderId].get();
    effectPtr->AddConnection(
        dynamic_cast<Controllable &>(folder->GetChild(cName)), cInputIndex);
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
    else if (t == "preset-value")
      ParsePresetValue(ToObj(control), management);
    else if (t == "effect")
      ParseEffect(ToObj(control), management);
  }
}

void ParseSourceValues(const Array &node, Management &management) {
  for (const Node &control : node) ParsePresetValue(ToObj(control), management);
}

KeySceneItem &ParseKeySceneItem(const Object &node, Scene &scene) {
  KeySceneItem *item = scene.AddKeySceneItem(ToNum(node["offset"]).AsDouble());
  item->SetLevel(GetKeySceneLevel(ToStr(node["level"])));
  return *item;
}

ControlSceneItem &ParseControlSceneItem(const Object &node, Scene &scene,
                                        Management &management) {
  const size_t folderId = ToNum(node["folder"]).AsSize();
  Folder *folder = management.Folders()[folderId].get();
  Controllable &controllable = static_cast<Controllable &>(
      folder->GetChild(ToStr(node["controllable-ref"])));
  ControlSceneItem *item = scene.AddControlSceneItem(
      ToNum(node["offset"]).AsDouble(), controllable, 0);
  item->StartValue().Set(ToNum(node["start-value"]).AsUInt());
  item->EndValue().Set(ToNum(node["end-value"]).AsUInt());
  return *item;
}

void ParseSceneItem(const Object &node, Scene &scene, Management &management) {
  const std::string &type = ToStr(node["type"]);
  SceneItem *item;
  if (type == "key")
    item = &ParseKeySceneItem(node, scene);
  else if (type == "control")
    item = &ParseControlSceneItem(node, scene, management);
  else
    throw std::runtime_error(std::string("Invalid type for scene item: ") +
                             type);
  item->SetDurationInMS(ToNum(node["duration"]).AsDouble());
}

void ParseScenes(const json::Array &node, Management &management) {
  for (const Node &item : node) {
    const Object &scene_node = ToObj(item);
    Scene *scene = management.GetShow().AddScene(false);
    ParseFolderAttr(scene_node, *scene, management);
    scene->SetAudioFile(ToStr(scene_node["audio-file"]));
    const Array &items = ToArr(scene_node["items"]);
    for (const Node &item_node : items) {
      ParseSceneItem(ToObj(item_node), *scene, management);
    }
  }
}

void ParseGUIPresetRef(const Object &node, gui::FaderSetupState &fader,
                       Management &management) {
  if (node.children.count("name")) {
    const size_t input = ToNum(node["input-index"]).AsSize();
    const size_t folderId = ToNum(node["folder"]).AsSize();
    const std::string name = ToStr(node["name"]);
    Folder *folder = management.Folders()[folderId].get();
    Controllable &controllable =
        static_cast<Controllable &>(folder->GetChild(name));
    fader.faders.emplace_back(management.GetSourceValue(controllable, input));
  } else {
    fader.faders.emplace_back(nullptr);
  }
  gui::FaderState &state = fader.faders.back();
  state.SetIsToggleButton(ToBool(node["is-toggle"]));
  if (state.IsToggleButton())
    state.SetNewToggleButtonColumn(ToBool(node["new-toggle-column"]));
}

void ParseGUIFaders(const Object &node, gui::GUIState &guiState,
                    Management &management) {
  guiState.FaderSetups().emplace_back(std::make_unique<gui::FaderSetupState>());
  gui::FaderSetupState &fader = *guiState.FaderSetups().back().get();
  fader.name = ToStr(node["name"]);
  fader.isActive = ToBool(node["active"]);
  fader.isSolo = ToBool(node["solo"]);
  fader.fadeInSpeed = ToNum(node["fade-in"]).AsSize();
  fader.fadeOutSpeed = ToNum(node["fade-out"]).AsSize();
  fader.width = ToNum(node["width"]).AsSize();
  fader.height = ToNum(node["height"]).AsSize();
  const Array &faders = ToArr(node["faders"]);
  for (const Node &item : faders) {
    const Object &fader_node = ToObj(item);
    ParseGUIPresetRef(fader_node, fader, management);
  }
}

void ParseGUI(const Object &node, gui::GUIState &guiState,
              Management &management) {
  const Array &states = ToArr(node["states"]);
  for (const Node &item : states) {
    const Object &state_node = ToObj(item);
    ParseGUIFaders(state_node, guiState, management);
  }
}

void parseGlightShow(const Object &node, Management &management,
                     gui::GUIState *guiState) {
  ParseFolders(ToArr(node["folders"]), management);
  ParseTheatre(ToObj(node["theatre"]), management);
  ParseControls(ToArr(node["controls"]), management);
  ParseSourceValues(ToArr(node["source-values"]), management);
  ParseScenes(ToArr(node["scenes"]), management);
  if (guiState != nullptr) {
    const auto &gui = node.children.find("gui");
    if (gui != node.children.end())
      ParseGUI(ToObj(*gui->second), *guiState, management);
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

}  // namespace glight::system
