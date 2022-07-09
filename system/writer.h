#ifndef WRITER_H
#define WRITER_H

#include <map>
#include <set>
#include <stdexcept>
#include <string>

#include "../theatre/management.h"

#include "jsonwriter.h"

class GUIState;

class WriterException : public std::runtime_error {
 public:
  WriterException(const std::string &msg) : std::runtime_error(msg) {}
};

/**
 * @author Andre Offringa
 */
class Writer {
 public:
  Writer(Management &management);
  ~Writer() {}

  void SetGUIState(GUIState &guiState) { _guiState = &guiState; }

  void Write(std::ostream &stream);
  void Write(const std::string &filename);

 private:
  void writeGlightShow();
  void writeFolders();

  void writeNameAttributes(const class NamedObject &obj);
  void writeFolderAttributes(const class FolderObject &obj);

  void writeDmxChannel(const class DmxChannel &dmxChannel);
  void writeFixtureType(const class FixtureType &fixtureType);
  void writeFixture(const class Fixture &fixture);
  void writeFixtureFunction(const class FixtureFunction &fixtureFunction);
  void writePresetCollection(const class PresetCollection &presetCollection);
  void writePresetValue(const class PresetValue &presetValue);
  void writeControllable(const class Controllable &controllable);
  void writeFixtureControl(const class FixtureControl &control);
  void writeChase(const class Chase &control);
  void writeTimeSequence(const class TimeSequence &control);
  void writeTransition(const class Transition &transition);
  void writeTrigger(const class Trigger &trigger);
  void writeSequence(const class Sequence &sequence);
  void writeEffect(const class Effect &effect);

  void writeScene(const class Scene &scene);
  void writeSceneItem(const class SceneItem &item);
  void writeKeySceneItem(const class KeySceneItem &item);
  void writeControlSceneItem(const class ControlSceneItem &item);

  void writeGUIState(const class GUIState &guiState);
  void writeFaderState(const class FaderSetupState &guiState);

  class Management &_management;
  class GUIState *_guiState;

  JsonWriter writer_;

  void requireControllable(const class Controllable &controllable) {
    writeControllable(controllable);
  }

  std::set<const class Controllable *> _controllablesWritten;
  std::map<const Folder *, size_t> _folderIds;
};

#endif
