#ifndef GLIGHT_THEATRE_MANAGEMENT_TOOLS_H_
#define GLIGHT_THEATRE_MANAGEMENT_TOOLS_H_

#include <vector>

#include "forwards.h"

namespace glight::theatre {

void SetAllFixtures(Management& management,
                    const std::vector<Fixture*> fixtures, const Color& color);

}

#endif
