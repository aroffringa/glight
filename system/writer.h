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

void Write(std::ostream &stream, Management &management, GUIState *guiState);
void Write(const std::string &filename, Management &management,
           GUIState *guiState);

#endif
