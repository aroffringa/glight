#include "system/settings.h"

#include <boost/test/unit_test.hpp>

namespace glight::system {

BOOST_AUTO_TEST_SUITE(settings)

BOOST_AUTO_TEST_CASE(load) {
  const Settings settings = LoadSettings();
  // Wherever this runs _may_ have a config file, so this test can only
  // check if loading doesn't crash and settings that are excepted to be
  // non-empty are in fact also non-empty.
  BOOST_CHECK(!settings.audio_input.empty());
  BOOST_CHECK(!settings.audio_output.empty());
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace glight::system
