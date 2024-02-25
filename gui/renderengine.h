#ifndef GLIGHT_RENDERING_H_
#define GLIGHT_RENDERING_H_

#include "../theatre/management.h"
#include "../theatre/position.h"
#include "../theatre/valuesnapshot.h"

#include <cairomm/context.h>

#include <vector>

namespace glight::gui {

struct DrawStyle {
  size_t xOffset;
  size_t yOffset;
  size_t width;
  size_t height;
  double timeSince;
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
                    const std::vector<theatre::Fixture *> &selected_fixtures);

  void DrawSelectionRectangle(const Cairo::RefPtr<Cairo::Context> &cairo,
                              const theatre::Position &from,
                              const theatre::Position &to) const;

  theatre::Fixture *FixtureAt(const theatre::Position &position) const;
  theatre::Fixture *FixtureAt(double mouse_x, double mouse_y, double width,
                              double height) const {
    return FixtureAt(MouseToPosition(mouse_x, mouse_y, width, height));
  }
  theatre::Position MouseToPosition(double mouse_x, double mouse_y,
                                    double width, double height) const;
  bool IsMoving() const { return is_moving_; }

 private:
  const theatre::Management &management_;
  std::vector<FixtureState> state_;
  double scale_ = 1.0;
  bool is_moving_ = false;
};

}  // namespace glight::gui

#endif
