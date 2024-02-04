#ifndef GLIGHT_SYSTEM_COLOR_MAP_H_
#define GLIGHT_SYSTEM_COLOR_MAP_H_

#include <cassert>
#include <vector>

#include <iostream>

#include "theatre/color.h"

namespace glight::system {

class ColorMap {
 public:
  ColorMap() = default;
  ColorMap(const std::vector<theatre::Color>& palette, int divisor)
      : divisor_(divisor) {
    assert(divisor > 1);
    const int limit = 1 + 256 / divisor;
    for (int red = 0; red != limit; ++red) {
      for (int green = 0; green != limit; ++green) {
        for (int blue = 0; blue != limit; ++blue) {
          // find closest color
          size_t closest = 0;
          unsigned closest_distance = 1024;
          for (size_t i = 0; i != palette.size(); ++i) {
            const unsigned d =
                std::abs(int(palette[i].Red()) - red * divisor) +
                std::abs(int(palette[i].Green()) - green * divisor) +
                std::abs(int(palette[i].Blue()) - blue * divisor);
            if (d <= closest_distance) {
              closest = i;
              closest_distance = d;
            }
          }
          map_.emplace_back(closest);
        }
      }
    }
  }

  unsigned short GetIndex(const theatre::Color& color) const {
    const int limit = 1 + 256 / divisor_;
    const int start = divisor_ / 2 - 1;
    const size_t unbounded_index =
        ((color.Red() + start) / divisor_) * limit * limit +
        ((color.Green() + start) / divisor_) * limit +
        (color.Blue() + start) / divisor_;
    const size_t index = std::min<size_t>(unbounded_index, map_.size());
    return map_[index];
  }

 private:
  int divisor_ = 0;
  // Maps a color to the palette index.
  // The color components have been divided by 8 to limit the
  // size of the map, and are indexed by r*divisor^2 + g*divisor + b;
  std::vector<unsigned short> map_;
};

}  // namespace glight::system

#endif
