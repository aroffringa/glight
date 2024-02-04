#include <boost/test/unit_test.hpp>

#include <memory>

#include "system/colormap.h"

using glight::system::ColorMap;
using glight::theatre::Color;

BOOST_AUTO_TEST_SUITE(color_map)

BOOST_AUTO_TEST_CASE(large_divider) {
  const ColorMap cm({Color{0, 0, 0}, Color{255, 255, 255}}, 128);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{0, 0, 0}), 0);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{255, 255, 255}), 1);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{64, 64, 64}), 0);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{192, 192, 192}), 1);
}

BOOST_AUTO_TEST_CASE(large_smaller) {
  const ColorMap cm({Color{0, 0, 0}, Color{255, 255, 255}}, 8);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{0, 0, 0}), 0);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{255, 255, 255}), 1);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{64, 64, 64}), 0);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{192, 192, 192}), 1);
}

BOOST_AUTO_TEST_CASE(many_choices) {
  const std::vector<Color> colors{
      Color{255, 255, 255}, Color{255, 128, 0}, Color{255, 0, 0},
      Color{200, 0, 0},     Color{200, 200, 0}, Color{110, 200, 0},
  };
  const ColorMap cm(colors, 8);
  for (size_t i = 0; i != colors.size(); ++i) {
    BOOST_CHECK_EQUAL(cm.GetIndex(colors[i]), i);
  }
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{0, 0, 0}), 3);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{1, 1, 1}), 3);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{100, 0, 0}), 3);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{230, 0, 0}), 2);
  BOOST_CHECK_EQUAL(cm.GetIndex(Color{100, 100, 0}), 5);
}

BOOST_AUTO_TEST_SUITE_END()
