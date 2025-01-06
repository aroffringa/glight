#ifndef GLIGHT_RENDERING_H_
#define GLIGHT_RENDERING_H_

#include "../theatre/coordinate2d.h"
#include "../theatre/management.h"
#include "../theatre/valuesnapshot.h"

#include "system/trackableptr.h"

#include <cairomm/context.h>

#include <vector>

namespace glight::gui {

/**
 * This struct holds the "cairo" screen sizes and other drawing info.
 */
struct DrawStyle {
  size_t x_offset = 0;
  size_t y_offset = 0;
  size_t width = 0;
  size_t height = 0;
  double time_since_previous = 0.0;
  bool draw_fixtures = true;
  bool draw_beams = true;
  bool draw_projections = true;
  bool draw_walls = true;
  bool draw_background = false;
};

struct FixtureState {
  double continuous_rotation = 0.0;
};

class RenderEngine {
 public:
  RenderEngine(const theatre::Management &management);

  void DrawSnapshot(const Cairo::RefPtr<Cairo::Context> &cairo,
                    const theatre::ValueSnapshot &snapshot,
                    const DrawStyle &style,
                    const std::vector<system::ObservingPtr<theatre::Fixture>>
                        &selected_fixtures);
  void DrawSelectedFixtures(
      const Cairo::RefPtr<Cairo::Context> &cairo,
      const std::vector<system::ObservingPtr<theatre::Fixture>>
          &selected_fixtures) const;
  void DrawSelectionRectangle(const Cairo::RefPtr<Cairo::Context> &cairo,
                              const theatre::Coordinate2D &from,
                              const theatre::Coordinate2D &to) const;

  system::ObservingPtr<theatre::Fixture> FixtureAt(
      const theatre::Coordinate2D &position) const;
  system::ObservingPtr<theatre::Fixture> FixtureAt(double mouse_x,
                                                   double mouse_y, double width,
                                                   double height) const {
    return FixtureAt(MouseToPosition(mouse_x, mouse_y, width, height));
  }
  system::ObservingPtr<theatre::Fixture> GetDirectionHandleAt(
      const std::vector<system::ObservingPtr<theatre::Fixture>> &fixtures,
      const theatre::Coordinate2D &position) const;
  theatre::Coordinate2D MouseToPosition(double mouse_x, double mouse_y,
                                        double width, double height) const;
  bool IsMoving() const { return is_moving_; }

 private:
  const theatre::Management &management_;
  std::vector<FixtureState> state_;
  double scale_ = 1.0;
  double x_padding_ = 0.0;
  double y_padding_ = 0.0;
  bool is_moving_ = false;
};

}  // namespace glight::gui

#endif
