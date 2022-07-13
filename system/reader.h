#ifndef READER_H_
#define READER_H_

#include <iostream>
#include <string>

class GUIState;
class Management;

void Read(const std::string &filename, Management &management,
          GUIState *guiState = nullptr);
void Read(std::istream &stream, Management &management,
          GUIState *guiState = nullptr);

#endif
