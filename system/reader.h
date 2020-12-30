#ifndef READER_H
#define READER_H

#include "../theatre/timesequence.h"

#include <stdexcept>
#include <string>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <glibmm/ustring.h>

/**
        @author Andre Offringa
*/
class Reader {
 public:
  Reader(class Management &management);

  void SetGUIState(class GUIState &guiState) { _guiState = &guiState; }

  void Read(const Glib::ustring &filename);

 private:
  std::string getStringAttribute(xmlNode *node, const char *name) const {
    xmlChar *typeCh = xmlGetProp(node, BAD_CAST name);
    if (typeCh == nullptr)
      throw std::runtime_error(std::string("Could not find attribute ") + name);
    std::string typeStr((const char *)typeCh);
    xmlFree(typeCh);
    return typeStr;
  }
  bool hasAttribute(xmlNode *node, const char *name) const {
    return xmlHasProp(node, BAD_CAST name) != NULL;
  }
  int getIntAttribute(xmlNode *node, const char *name) const;
  bool getBoolAttribute(xmlNode *node, const char *name) const {
    return getIntAttribute(node, name) != 0;
  }
  double getDoubleAttribute(xmlNode *node, const char *name) const;
  std::string name(xmlNode *node) {
    return std::string((const char *)node->name);
  }

  void parseNameAttr(xmlNode *node, class NamedObject &object);
  void parseFolderAttr(xmlNode *node, class FolderObject &object,
                       bool hasFolder = true);

  void parseGlightShow(xmlNode *node);
  void parseGroup(xmlNode *node);
  void parseFolders(xmlNode *node);
  void parseTheatre(xmlNode *node);
  void parseTheatreItem(xmlNode *node);
  void parseControl(xmlNode *node);
  void parseControlItem(xmlNode *node);
  void parseFixtureType(xmlNode *node);
  void parseFixture(xmlNode *node);
  void parseFixtureControl(xmlNode *node);
  void parsePresetCollection(xmlNode *node);
  void parseSequence(xmlNode *node, class Sequence &sequence);
  void parseChase(xmlNode *node);
  void parseTimeSequence(xmlNode *node);
  void parseTimeSequenceStep(xmlNode *node, TimeSequence::Step &step);
  void parsePresetValue(xmlNode *node);
  void parseEffect(xmlNode *node);

  void parseFixtureFunction(xmlNode *node, class Fixture &parentFixture);
  void parseDmxChannel(xmlNode *node, class DmxChannel &dmxChannel);
  void parseTrigger(xmlNode *node, class Trigger &trigger);
  void parseTransition(xmlNode *node, class Transition &transition);

  void parseShow(xmlNode *node);
  void parseShowItem(xmlNode *node);
  void parseScene(xmlNode *node);
  void parseSceneItem(xmlNode *node, class Scene &scene);

  class KeySceneItem &parseKeySceneItem(xmlNode *node, class Scene &scene);
  class ControlSceneItem &parseControlSceneItem(xmlNode *node,
                                                class Scene &scene);

  void parseGUI(xmlNode *node, class GUIState &guiState);
  void parseGUIItem(xmlNode *node, class GUIState &guiState);
  void parseGUIFaders(xmlNode *node, class GUIState &guiState);
  void parseGUIPresetRef(xmlNode *node, class FaderSetupState &faders);

  xmlDocPtr _xmlDocument;
  class Management &_management;
  class Theatre &_theatre;
  class GUIState *_guiState;
};

#endif
