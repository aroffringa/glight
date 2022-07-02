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

Writer::Writer(Management &management)
    : _management(management), _guiState(nullptr) {}

void Writer::Write(const std::string &filename) {
  _controllablesWritten.clear();
  _folderIds.clear();

  std::ofstream file(filename);
  writer_ = JsonWriter(file);

  writeGlightShow();
}

void Writer::writeGlightShow() {
  writer_.StartObject("glight-show");

  writeFolders();

  writer_.StartObject("theatre");

  Theatre &theatre = _management.GetTheatre();

  const std::vector<std::unique_ptr<FixtureType>> &fixtureTypes =
      theatre.FixtureTypes();
  for (const std::unique_ptr<FixtureType> &ft : fixtureTypes)
    writeFixtureType(*ft);

  const std::vector<std::unique_ptr<Fixture>> &fixtures = theatre.Fixtures();
  for (const std::unique_ptr<Fixture> &f : fixtures) writeFixture(*f);

  writer_.EndObject();  // theatre

  writer_.StartObject("control");

  const std::vector<std::unique_ptr<Controllable>> &controllables =
      _management.Controllables();
  for (const std::unique_ptr<Controllable> &c : controllables)
    writeControllable(*c);

  const std::vector<std::unique_ptr<SourceValue>> &sourceValues =
      _management.SourceValues();
  for (const std::unique_ptr<SourceValue> &sv : sourceValues)
    writePresetValue(sv->Preset());

  writer_.EndObject();  // control

  writer_.StartObject("show");

  Show &show = _management.GetShow();

  const std::vector<std::unique_ptr<Scene>> &scenes = show.Scenes();
  for (const std::unique_ptr<Scene> &scene : scenes) writeScene(*scene);

  writer_.EndObject();  // show

  writer_.StartObject("gui");

  if (_guiState != nullptr) writeGUIState(*_guiState);

  writer_.EndObject();  // gui

  writer_.EndObject();  // glight-show
}

void Writer::writeFolders() {
  writer_.StartArray("folders");
  for (size_t i = 0; i != _management.Folders().size(); ++i) {
    const Folder &folder = *_management.Folders()[i];
    _folderIds.emplace(&folder, i);
    writer_.StartObject();
    writer_.Number("id", i);
    writer_.String("name", folder.Name());
    if (!folder.IsRoot())
      writer_.Number("parent", _folderIds.find(&folder.Parent())->second);
    writer_.EndObject();
  }
  writer_.EndArray();
}

void Writer::writeNameAttributes(const NamedObject &obj) {
  writer_.String("name", obj.Name());
}

void Writer::writeFolderAttributes(const FolderObject &obj) {
  writeNameAttributes(obj);
  writer_.Number("parent", _folderIds.find(&obj.Parent())->second);
}

void Writer::writeFixture(const Fixture &fixture) {
  writer_.StartObject("fixture");
  writeNameAttributes(fixture);
  writer_.String("type", fixture.Type().Name());
  writer_.Number("position-x", fixture.GetPosition().X());
  writer_.Number("position-y", fixture.GetPosition().Y());
  writer_.String("symbol", fixture.Symbol().Name());
  const std::vector<std::unique_ptr<FixtureFunction>> &functions =
      fixture.Functions();
  for (const std::unique_ptr<FixtureFunction> &ff : functions)
    writeFixtureFunction(*ff);
  writer_.EndObject();
}

void Writer::writeFixtureFunction(const FixtureFunction &fixtureFunction) {
  writer_.StartObject("fixture-function");
  writer_.String("name", fixtureFunction.Name());
  writer_.String("type", FunctionTypeDescription(fixtureFunction.Type()));
  writeDmxChannel(fixtureFunction.FirstChannel());
  writer_.EndObject();
}

void Writer::writeDmxChannel(const DmxChannel &dmxChannel) {
  writer_.StartObject("dmx-channel");
  writer_.Number("universe", dmxChannel.Universe());
  writer_.Number("channel", dmxChannel.Channel());
  writer_.String("default-mix-style", ToString(dmxChannel.DefaultMixStyle()));
  writer_.EndObject();
}

void Writer::writeFixtureType(const FixtureType &fixtureType) {
  writer_.StartObject("fixture-type");
  writeFolderAttributes(fixtureType);
  writer_.String("fixture-class",
                 FixtureType::ClassName(fixtureType.GetFixtureClass()));
  writer_.EndObject();
}

void Writer::writeControllable(const Controllable &controllable) {
  if (_controllablesWritten.count(&controllable) == 0) {
    const FixtureControl *fixtureControl =
        dynamic_cast<const FixtureControl *>(&controllable);
    const Chase *chase = dynamic_cast<const Chase *>(&controllable);
    const TimeSequence *tSequence =
        dynamic_cast<const TimeSequence *>(&controllable);
    const PresetCollection *presetCollection =
        dynamic_cast<const PresetCollection *>(&controllable);
    const Effect *effect = dynamic_cast<const Effect *>(&controllable);

    if (fixtureControl)
      writeFixtureControl(*fixtureControl);
    else if (chase)
      writeChase(*chase);
    else if (tSequence)
      writeTimeSequence(*tSequence);
    else if (presetCollection)
      writePresetCollection(*presetCollection);
    else if (effect)
      writeEffect(*effect);
    else
      throw std::runtime_error("Unknown controllable");
  }
  _controllablesWritten.insert(&controllable);
}

void Writer::writePresetCollection(
    const class PresetCollection &presetCollection) {
  const std::vector<std::unique_ptr<PresetValue>> &values =
      presetCollection.PresetValues();
  for (const std::unique_ptr<PresetValue> &pv : values)
    requireControllable(pv->Controllable());

  writer_.Name("preset-collection");
  writer_.StartObject();
  writeFolderAttributes(presetCollection);
  for (const std::unique_ptr<PresetValue> &pv : values) writePresetValue(*pv);
  writer_.EndObject();
}

void Writer::writePresetValue(const PresetValue &presetValue) {
  requireControllable(presetValue.Controllable());

  writer_.StartObject("preset-value");
  writer_.String("controllable-ref", presetValue.Controllable().Name());
  writer_.Number("input-index", presetValue.InputIndex());
  writer_.Number("folder", _folderIds[&presetValue.Controllable().Parent()]);
  writer_.Number("value", presetValue.Value().UInt());
  writer_.EndObject();
}

void Writer::writeFixtureControl(const FixtureControl &control) {
  writer_.StartObject("fixture-control");
  writeFolderAttributes(control);
  writer_.String("fixture-ref", control.Fixture().Name());
  writer_.EndObject();
}

void Writer::writeChase(const Chase &chase) {
  const std::vector<std::pair<Controllable *, size_t>> &list =
      chase.Sequence().List();
  for (const std::pair<Controllable *, size_t> &input : list)
    requireControllable(*input.first);

  writer_.StartObject("chase");
  writeFolderAttributes(chase);
  writeTrigger(chase.Trigger());
  writeTransition(chase.Transition());
  writeSequence(chase.Sequence());
  writer_.EndObject();
}

void Writer::writeTimeSequence(const TimeSequence &timeSequence) {
  const std::vector<std::pair<Controllable *, size_t>> &list =
      timeSequence.Sequence().List();
  for (const std::pair<Controllable *, size_t> &input : list)
    requireControllable(*input.first);

  writer_.StartObject("time-sequence");
  writeFolderAttributes(timeSequence);
  writer_.Boolean("sustain", timeSequence.Sustain());
  writer_.Number("repeat-count", timeSequence.RepeatCount());
  writeSequence(timeSequence.Sequence());
  writer_.StartArray("steps");
  for (size_t i = 0; i != timeSequence.Size(); ++i) {
    const TimeSequence::Step &step = timeSequence.GetStep(i);
    writer_.StartObject();
    writeTrigger(step.trigger);
    writeTransition(step.transition);
    writer_.EndObject();
  }
  writer_.EndArray();   // steps
  writer_.EndObject();  // time-sequence
}

void Writer::writeTrigger(const Trigger &trigger) {
  writer_.StartObject("trigger");
  writer_.String("type", ToString(trigger.Type()));
  writer_.Number("delay-in-ms", trigger.DelayInMs());
  writer_.Number("delay-in-beats", trigger.DelayInBeats());
  writer_.Number("delay-in-syncs", trigger.DelayInSyncs());
  writer_.EndObject();
}

void Writer::writeTransition(const Transition &transition) {
  writer_.StartObject("transition");
  writer_.Number("length-in-ms", transition.LengthInMs());
  writer_.String("type", ToString(transition.Type()));
  writer_.EndObject();
}

void Writer::writeSequence(const Sequence &sequence) {
  writer_.StartObject("sequence");
  for (const std::pair<Controllable *, size_t> &input : sequence.List()) {
    writer_.StartObject("input-ref");
    writer_.Number("input-index", input.second);
    writer_.Number("folder", _folderIds[&input.first->Parent()]);
    writer_.String("name", input.first->Name());
    writer_.EndObject();
  }
  writer_.EndObject();
}

void Writer::writeEffect(const class Effect &effect) {
  if (_controllablesWritten.count(&effect) == 0) {
    for (const std::pair<Controllable *, size_t> &c : effect.Connections())
      requireControllable(*c.first);

    writer_.StartObject("effect");
    writeFolderAttributes(effect);
    writer_.String("type", effect.TypeToName(effect.GetType()));
    std::unique_ptr<PropertySet> ps = PropertySet::Make(effect);

    // the number and name of the effect controls are implied from the
    // effect type, so do not require to be stored.

    for (const Property &p : *ps) {
      writer_.StartObject("property");
      writer_.String("name", p.Name());
      switch (p.GetType()) {
        case Property::Choice:
          writer_.String("value", ps->GetChoice(p));
          break;
        case Property::ControlValue:
          writer_.Number("value", ps->GetControlValue(p));
          break;
        case Property::Duration:
          writer_.Number("value", ps->GetDuration(p));
          break;
        case Property::Boolean:
          writer_.Boolean("value", ps->GetBool(p));
          break;
        case Property::Integer:
          writer_.Number("value", ps->GetInteger(p));
          break;
      }
      writer_.EndObject();
    }
    for (const std::pair<Controllable *, size_t> &c : effect.Connections()) {
      writer_.StartObject("connection-ref");
      writer_.Number("input-index", c.second);
      writer_.Number("folder", _folderIds[&c.first->Parent()]);
      writer_.String("name", c.first->Name());
      writer_.EndObject();
    }
    writer_.EndObject();
    _controllablesWritten.insert(&effect);
  }
}

void Writer::writeScene(const Scene &scene) {
  writer_.StartObject("scene");

  writeFolderAttributes(scene);
  writer_.String("audio-file", scene.AudioFile());

  const std::multimap<double, std::unique_ptr<SceneItem>> &items =
      scene.SceneItems();
  for (const std::pair<const double, std::unique_ptr<SceneItem>> &sceneItem :
       items) {
    writeSceneItem(*sceneItem.second);
  }

  writer_.EndObject();
}

void Writer::writeSceneItem(const SceneItem &item) {
  writer_.StartObject("scene-item");

  writer_.Number("offset", item.OffsetInMS());
  writer_.Number("duration", item.DurationInMS());

  const KeySceneItem *keyItem = dynamic_cast<const KeySceneItem *>(&item);
  const ControlSceneItem *controlItem =
      dynamic_cast<const ControlSceneItem *>(&item);

  if (keyItem != nullptr)
    writeKeySceneItem(*keyItem);
  else
    writeControlSceneItem(*controlItem);

  writer_.EndObject();
}

void Writer::writeKeySceneItem(const KeySceneItem &item) {
  writer_.String("type", "key");
  writer_.String("level", ToString(item.Level()));
}

void Writer::writeControlSceneItem(const ControlSceneItem &item) {
  writer_.String("type", "control");
  writer_.Number("start-value", item.StartValue().UInt());
  writer_.Number("end-value", item.EndValue().UInt());
  writer_.String("controllable-ref", item.Controllable().Name());
  writer_.Number("folder", _folderIds[&item.Controllable().Parent()]);
}

void Writer::writeGUIState(const GUIState &guiState) {
  for (const std::unique_ptr<FaderSetupState> &fState : guiState.FaderSetups())
    writeFaderState(*fState);
}

void Writer::writeFaderState(const FaderSetupState &guiState) {
  writer_.StartObject("faders");
  writer_.String("name", guiState.name);
  writer_.Boolean("active", guiState.isActive);
  writer_.Boolean("solo", guiState.isSolo);
  writer_.Number("fade-in", guiState.fadeInSpeed);
  writer_.Number("fade-out", guiState.fadeOutSpeed);
  writer_.Number("width", guiState.width);
  writer_.Number("height", guiState.height);
  writer_.StartArray("list");
  for (const FaderState &fader : guiState.faders) {
    writer_.StartObject();
    writer_.Boolean("is-toggle", fader.IsToggleButton());
    if (fader.IsToggleButton())
      writer_.Boolean("new-toggle-column", fader.NewToggleButtonColumn());
    if (fader.GetSourceValue() != nullptr) {
      writer_.Number("input-index",
                     fader.GetSourceValue()->Preset().InputIndex());
      writer_.Number(
          "folder",
          _folderIds[&fader.GetSourceValue()->Controllable().Parent()]);
      writer_.String("name", fader.GetSourceValue()->Controllable().Name());
    }
  }
  writer_.EndArray();
  writer_.EndObject();
}
