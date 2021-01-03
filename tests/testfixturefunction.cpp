#include "../theatre/fixturefunction.h"
#include "../theatre/theatre.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(fixture_function)

BOOST_AUTO_TEST_CASE(MixChannels_8bit) {
  Theatre theatre;
  FixtureFunction ff(theatre, FunctionType::Red, "ff test");
  ff.SetChannel(DmxChannel(3, 0), false);
  std::vector<unsigned> channelValues(512, 0);
  ff.MixChannels(1 << 8, MixStyle::HighestValue, channelValues.data(), 0);

  BOOST_CHECK_EQUAL(channelValues[3], 1 << 8);
}

BOOST_AUTO_TEST_CASE(MixChannels_16bit) {
  Theatre theatre;
  FixtureFunction ff(theatre, FunctionType::Red, "ff test");
  ff.SetChannel(DmxChannel(3, 0), true);
  std::vector<unsigned> channelValues(512, 0);
  ff.MixChannels(1 << 8, MixStyle::HighestValue, channelValues.data(), 0);

  BOOST_CHECK_EQUAL(channelValues[3], 0);
  BOOST_CHECK_EQUAL(channelValues[4], 1 << 16);

  channelValues.assign(512, 0);
  ff.MixChannels((1 << 24) - 1, MixStyle::HighestValue, channelValues.data(),
                 0);
  BOOST_CHECK_EQUAL(channelValues[3], ((1 << 24) - 1) & 0xFF0000);
  BOOST_CHECK_EQUAL(channelValues[4], (((1 << 24) - 1) & 0x00FFFF) << 8);
}

BOOST_AUTO_TEST_SUITE_END()
