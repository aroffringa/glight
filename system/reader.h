#ifndef READER_H_
#define READER_H_

#include "jsonreader.h"

#include "../theatre/timesequence.h"

#include <stdexcept>
#include <string>

class GUIState;
class Management;

/**
 * @author Andre Offringa
 */
class Reader {
 public:
  Reader(Management &management);

  void SetGUIState(GUIState &guiState) { _guiState = &guiState; }

  void Read(const std::string &filename);
  void Read(std::istream &stream);

 private:
  void parseGlightShow(const json::Object &node);

  Management &_management;
  GUIState *_guiState;
};

#endif
