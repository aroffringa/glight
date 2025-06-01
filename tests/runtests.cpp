#define BOOST_TEST_MODULE glight
// #define BOOST_TEST_DYN_LINK

#include <boost/test/included/unit_test.hpp>

#include <alsa/asoundlib.h>

struct LibASoundDestruction {
  void teardown() { snd_config_update_free_global(); }
};

BOOST_TEST_GLOBAL_FIXTURE(LibASoundDestruction);
