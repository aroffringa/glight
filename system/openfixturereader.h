#ifndef SYSTEM_OPEN_FIXTURE_READER_H
#define SYSTEM_OPEN_FIXTURE_READER_H

#include "jsonreader.h"

#include "../theatre/fixturemode.h"
#include "../theatre/management.h"

namespace glight::system {

void ReadOpenFixture(theatre::Management& theatre, const json::Node& node);

}

#endif
