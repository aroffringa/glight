#include "reader.h"

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

#include "../gui/guistate.h"

Reader::Reader(Management &management)
    : _management(management),
      _theatre(management.Theatre()),
      _guiState(nullptr) {}

void Reader::Read(const Glib::ustring &filename) {
  _xmlDocument = xmlReadFile(filename.c_str(), NULL, 0);
  if (_xmlDocument == NULL) throw std::runtime_error("Failed to parse file");

  xmlNode *rootElement = xmlDocGetRootElement(_xmlDocument);

  for (xmlNode *curNode = rootElement; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      parseGlightShow(curNode);
    }
  }

  xmlFreeDoc(_xmlDocument);
}

int Reader::getIntAttribute(xmlNode *node, const char *name) const {
  return atoi(getStringAttribute(node, name).c_str());
}

double Reader::getDoubleAttribute(xmlNode *node, const char *name) const {
  return atof(getStringAttribute(node, name).c_str());
}

void Reader::parseGlightShow(xmlNode *node) {
  if (name(node) != "glight-show")
    throw std::runtime_error("File is not a compatible glight show save file");

  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) parseGroup(curNode);
  }
}

void Reader::parseGroup(xmlNode *node) {
  if (name(node) == "folders")
    parseFolders(node);
  else if (name(node) == "theatre")
    parseTheatre(node);
  else if (name(node) == "control")
    parseControl(node);
  else if (name(node) == "show")
    parseShow(node);
  else if (name(node) == "gui") {
    if (_guiState == nullptr) {
      GUIState scratchState;
      parseGUI(node, scratchState);
    } else
      parseGUI(node, *_guiState);
  } else
    throw std::runtime_error(std::string("Invalid node: ") + name(node));
}

void Reader::parseNameAttr(xmlNode *node, class NamedObject &object) {
  object.SetName(getStringAttribute(node, "name"));
}

void Reader::parseFolderAttr(xmlNode *node, class FolderObject &object,
                             bool hasFolder) {
  if (hasFolder) {
    size_t parent = getIntAttribute(node, "parent");
    _management.Folders()[parent]->Add(object);
  }
  parseNameAttr(node, object);
}

void Reader::parseFolders(xmlNode *node) {
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      if (name(curNode) == "folder") {
        if (hasAttribute(curNode, "parent")) {
          size_t parent = getIntAttribute(curNode, "parent");
          _management.AddFolder(*_management.Folders()[parent],
                                getStringAttribute(curNode, "name"));
        } else {
          _management.RootFolder().SetName(getStringAttribute(curNode, "name"));
        }
      } else
        throw std::runtime_error(
            std::string("Invalid node while expecting a folder : ") +
            name(curNode));
    }
  }
}

void Reader::parseTheatre(xmlNode *node) {
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) parseTheatreItem(curNode);
  }
}

void Reader::parseControl(xmlNode *node) {
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) parseControlItem(curNode);
  }
}

void Reader::parseTheatreItem(xmlNode *node) {
  const std::string n = name(node);
  if (n == "fixture-type")
    parseFixtureType(node);
  else if (n == "fixture")
    parseFixture(node);
  else
    throw std::runtime_error(
        std::string("Invalid node while expecting a theatre item : ") + n);
}

void Reader::parseControlItem(xmlNode *node) {
  const std::string n = name(node);
  if (n == "fixture-control")
    parseFixtureControl(node);
  else if (n == "preset-collection")
    parsePresetCollection(node);
  else if (n == "chase")
    parseChase(node);
  else if (n == "time-sequence")
    parseTimeSequence(node);
  else if (n == "preset-value")
    parsePresetValue(node);
  else if (n == "effect")
    parseEffect(node);
  else
    throw std::runtime_error(
        std::string("Invalid node while expecting a control item: ") + n);
}

void Reader::parseFixtureType(xmlNode *node) {
  enum FixtureType::FixtureClass cl =
      (enum FixtureType::FixtureClass)getIntAttribute(node, "fixture-class");
  FixtureType *type = dynamic_cast<FixtureType *>(
      _management.RootFolder().GetChildIfExists(FixtureType::ClassName(cl)));
  if (!type) {
    type = &_management.Theatre().AddFixtureType(
        cl);  // TODO we shouldn't use a type by its name, types should be
              // editable etc
    parseFolderAttr(node, *type);
  }
}

void Reader::parseFixture(xmlNode *node) {
  FixtureType &type = _theatre.GetFixtureType(getStringAttribute(node, "type"));
  Fixture &fixture = _theatre.AddFixture(type);
  parseNameAttr(node, fixture);
  fixture.Position().X() = getDoubleAttribute(node, "position-x");
  fixture.Position().Y() = getDoubleAttribute(node, "position-y");
  if (hasAttribute(node, "symbol"))
    fixture.SetSymbol(FixtureSymbol(getStringAttribute(node, "symbol")));
  fixture.ClearFunctions();

  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      if (name(curNode) == "fixture-function")
        parseFixtureFunction(curNode, fixture);
      else
        throw std::runtime_error("Invalid node in fixture");
    }
  }
}

void Reader::parseFixtureFunction(xmlNode *node, Fixture &parentFixture) {
  FixtureFunction &function =
      parentFixture.AddFunction((FunctionType)getIntAttribute(node, "type"));
  parseNameAttr(node, function);
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      if (name(curNode) == "dmx-channel") {
        DmxChannel channel;
        parseDmxChannel(curNode, channel);
        function.SetChannel(channel);
      } else
        throw std::runtime_error("Invalid node in fixture");
    }
  }
}

void Reader::parseDmxChannel(xmlNode *node, class DmxChannel &dmxChannel) {
  dmxChannel.SetUniverse(getIntAttribute(node, "universe"));
  dmxChannel.SetChannel(getIntAttribute(node, "channel"));
  dmxChannel.SetDefaultMixStyle(
      (ControlValue::MixStyle)getIntAttribute(node, "default-mix-style"));
}

void Reader::parseFixtureControl(xmlNode *node) {
  Fixture &fixture =
      _theatre.GetFixture(getStringAttribute(node, "fixture-ref"));
  FixtureControl &control = _management.AddFixtureControl(fixture);
  parseFolderAttr(node, control);
}

void Reader::parsePresetCollection(xmlNode *node) {
  PresetCollection &collection = _management.AddPresetCollection();
  parseFolderAttr(node, collection);
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      if (name(curNode) == "preset-value") {
        Folder &folder =
            *_management.Folders()[getIntAttribute(curNode, "folder")];
        FolderObject &obj =
            folder.GetChild(getStringAttribute(curNode, "controllable-ref"));
        size_t inputIndex = getIntAttribute(curNode, "input-index");
        Controllable *controllable = dynamic_cast<Controllable *>(&obj);
        if (controllable == nullptr)
          throw std::runtime_error(
              "Expecting a controllable in "
              "controllable-ref, but object named " +
              obj.Name() + " in folder " + folder.Name() +
              " is something different");
        PresetValue &value =
            collection.AddPresetValue(*controllable, inputIndex);
        value.SetValue(ControlValue(getIntAttribute(curNode, "value")));
      } else
        throw std::runtime_error("Bad node " + name(curNode) +
                                 " in preset collection");
    }
  }
}

void Reader::parseSequence(xmlNode *node, Sequence &sequence) {
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      if (name(curNode) == "input-ref") {
        size_t input = getIntAttribute(curNode, "input-index");
        size_t folderId = getIntAttribute(curNode, "folder");
        Controllable &c = dynamic_cast<Controllable &>(
            _management.Folders()[folderId]->GetChild(
                getStringAttribute(curNode, "name")));
        sequence.Add(c, input);
      } else
        throw std::runtime_error("Bad node in sequence");
    }
  }
}

void Reader::parseChase(xmlNode *node) {
  Chase &chase = _management.AddChase();
  parseFolderAttr(node, chase);
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      if (name(curNode) == "trigger")
        parseTrigger(curNode, chase.Trigger());
      else if (name(curNode) == "transition")
        parseTransition(curNode, chase.Transition());
      else if (name(curNode) == "sequence")
        parseSequence(curNode, chase.Sequence());
      else
        throw std::runtime_error("Bad node " + name(curNode) + " in chase");
    }
  }
}

void Reader::parseTimeSequence(xmlNode *node) {
  TimeSequence &timeSequence = _management.AddTimeSequence();
  parseFolderAttr(node, timeSequence);
  timeSequence.SetSustain(getBoolAttribute(node, "sustain"));
  timeSequence.SetRepeatCount(getIntAttribute(node, "repeat-count"));

  size_t stepIndex = 0;
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      if (name(curNode) == "sequence") {
        parseSequence(curNode, timeSequence.Sequence());
      } else if (name(curNode) == "step") {
        timeSequence.Steps().emplace_back();
        parseTimeSequenceStep(curNode, timeSequence.Steps().back());
        ++stepIndex;
      } else
        throw std::runtime_error("Bad node " + name(curNode) +
                                 " in time-sequence");
    }
  }
  if (timeSequence.Steps().size() != timeSequence.Sequence().Size())
    throw std::runtime_error(
        "nr of steps in time sequence doesn't match sequence size");
}

void Reader::parseTimeSequenceStep(xmlNode *node, TimeSequence::Step &step) {
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      if (name(curNode) == "trigger")
        parseTrigger(curNode, step.trigger);
      else if (name(curNode) == "transition")
        parseTransition(curNode, step.transition);
      else
        throw std::runtime_error("Bad node " + name(curNode) +
                                 " in time-sequence step");
    }
  }
}

void Reader::parsePresetValue(xmlNode *node) {
  size_t folderId = getIntAttribute(node, "folder");
  const std::string name = getStringAttribute(node, "controllable-ref");
  Folder *folder = _management.Folders()[folderId].get();
  Controllable &controllable =
      static_cast<Controllable &>(folder->GetChild(name));
  size_t inputIndex = getIntAttribute(node, "input-index");
  SourceValue &value = _management.AddSourceValue(controllable, inputIndex);
  value.Preset().SetValue(ControlValue(getIntAttribute(node, "value")));
}

void Reader::parseEffect(xmlNode *node) {
  Effect::Type type = Effect::NameToType(getStringAttribute(node, "type"));
  std::unique_ptr<Effect> effect = Effect::Make(type);
  parseFolderAttr(node, *effect);
  Effect *effectPtr = &_management.AddEffect(std::move(effect));
  std::unique_ptr<PropertySet> ps = PropertySet::Make(*effectPtr);
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      if (name(curNode) == "property") {
        std::string propName = getStringAttribute(curNode, "name");
        Property &p = ps->GetProperty(propName);
        switch (p.GetType()) {
          case Property::Choice:
            ps->SetChoice(p, getStringAttribute(curNode, "value"));
            break;
          case Property::ControlValue:
            ps->SetControlValue(p, getIntAttribute(curNode, "value"));
            break;
          case Property::Duration:
            ps->SetDuration(p, getDoubleAttribute(curNode, "value"));
            break;
          case Property::Boolean:
            ps->SetBool(p, getBoolAttribute(curNode, "value"));
            break;
          case Property::Integer:
            ps->SetInteger(p, getIntAttribute(curNode, "value"));
            break;
        }
      } else if (name(curNode) == "connection-ref") {
        std::string cName = getStringAttribute(curNode, "name");
        size_t cInputIndex = getIntAttribute(curNode, "input-index");
        size_t folderId = getIntAttribute(curNode, "folder");
        Folder *folder = _management.Folders()[folderId].get();
        effectPtr->AddConnection(
            static_cast<Controllable &>(folder->GetChild(cName)), cInputIndex);
      } else
        throw std::runtime_error("Bad element in effect");
    }
  }
}

void Reader::parseTrigger(xmlNode *node, class Trigger &trigger) {
  trigger.SetType((enum Trigger::Type)getIntAttribute(node, "type"));
  trigger.SetDelayInMs(getDoubleAttribute(node, "delay-in-ms"));
  trigger.SetDelayInBeats(getDoubleAttribute(node, "delay-in-beats"));
  trigger.SetDelayInSyncs(getDoubleAttribute(node, "delay-in-syncs"));
}

void Reader::parseTransition(xmlNode *node, class Transition &transition) {
  transition.SetType((enum Transition::Type)getIntAttribute(node, "type"));
  transition.SetLengthInMs(getDoubleAttribute(node, "length-in-ms"));
}

void Reader::parseShow(xmlNode *node) {
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) parseShowItem(curNode);
  }
}

void Reader::parseShowItem(xmlNode *node) {
  const std::string n = name(node);
  if (n == "scene")
    parseScene(node);
  else
    throw std::runtime_error(
        std::string("Invalid node while expecting a show item: ") + n);
}

void Reader::parseScene(xmlNode *node) {
  Scene *scene = _management.Show().AddScene();
  parseFolderAttr(node, *scene);
  scene->SetAudioFile(getStringAttribute(node, "audio-file"));

  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      if (name(curNode) == "scene-item")
        parseSceneItem(curNode, *scene);
      else
        throw std::runtime_error(
            std::string("Invalid node while expecting a scene item: ") +
            name(curNode));
    }
  }
}

void Reader::parseSceneItem(xmlNode *node, Scene &scene) {
  std::string type = getStringAttribute(node, "type");
  SceneItem *item;
  if (type == "key")
    item = &parseKeySceneItem(node, scene);
  else if (type == "control")
    item = &parseControlSceneItem(node, scene);
  else
    throw std::runtime_error(std::string("Invalid type for scene item: ") +
                             type);
  item->SetDurationInMS(getDoubleAttribute(node, "duration"));
}

KeySceneItem &Reader::parseKeySceneItem(xmlNode *node, Scene &scene) {
  KeySceneItem *item =
      scene.AddKeySceneItem(getDoubleAttribute(node, "offset"));
  item->SetLevel((enum KeySceneItem::Level)getIntAttribute(node, "level"));
  return *item;
}

ControlSceneItem &Reader::parseControlSceneItem(xmlNode *node, Scene &scene) {
  size_t folderId = getIntAttribute(node, "folder");
  Folder *folder = _management.Folders()[folderId].get();
  Controllable &controllable = static_cast<Controllable &>(
      folder->GetChild(getStringAttribute(node, "controllable-ref")));
  ControlSceneItem *item = scene.AddControlSceneItem(
      getDoubleAttribute(node, "offset"), controllable, 0);
  item->StartValue().Set(getIntAttribute(node, "start-value"));
  item->EndValue().Set(getIntAttribute(node, "end-value"));
  return *item;
}

void Reader::parseGUI(xmlNode *node, GUIState &guiState) {
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) parseGUIItem(curNode, guiState);
  }
}

void Reader::parseGUIItem(xmlNode *node, GUIState &guiState) {
  const std::string n = name(node);
  if (n == "faders")
    parseGUIFaders(node, guiState);
  else
    throw std::runtime_error("Invalid GUI element: " + n);
}

void Reader::parseGUIFaders(xmlNode *node, GUIState &guiState) {
  guiState.FaderSetups().emplace_back(new FaderSetupState());
  FaderSetupState &fader = *guiState.FaderSetups().back().get();
  fader.name = getStringAttribute(node, "name");
  fader.isActive = getBoolAttribute(node, "active");
  fader.isSolo = getBoolAttribute(node, "solo");
  if (hasAttribute(node, "fade-in"))
    fader.fadeInSpeed = getIntAttribute(node, "fade-in");
  if (hasAttribute(node, "fade-out"))
    fader.fadeOutSpeed = getIntAttribute(node, "fade-out");
  fader.width = getIntAttribute(node, "width");
  fader.height = getIntAttribute(node, "height");
  for (xmlNode *curNode = node->children; curNode != NULL;
       curNode = curNode->next) {
    if (curNode->type == XML_ELEMENT_NODE) {
      const std::string n = name(curNode);
      if (n == "fader")
        parseGUIPresetRef(curNode, fader);
      else
        throw std::runtime_error("Invalid GUI fader element: " + n);
    }
  }
}

void Reader::parseGUIPresetRef(xmlNode *node, FaderSetupState &fader) {
  if (hasAttribute(node, "name")) {
    size_t input = getIntAttribute(node, "input-index");
    size_t folderId = getIntAttribute(node, "folder");
    const std::string name = getStringAttribute(node, "name");
    Folder *folder = _management.Folders()[folderId].get();
    Controllable &controllable =
        static_cast<Controllable &>(folder->GetChild(name));
    fader.faders.emplace_back(_management.GetSourceValue(controllable, input));
  } else {
    fader.faders.emplace_back(nullptr);
  }
  FaderState &state = fader.faders.back();
  if (hasAttribute(node, "is-toggle"))
    state.SetIsToggleButton(getBoolAttribute(node, "is-toggle"));
  if (hasAttribute(node, "new-toggle-column"))
    state.SetNewToggleButtonColumn(getBoolAttribute(node, "new-toggle-column"));
}
