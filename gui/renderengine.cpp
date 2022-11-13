#include "renderengine.h"

#include "../theatre/fixture.h"
#include "../theatre/theatre.h"

namespace glight::gui {

namespace {
constexpr double radius(theatre::FixtureSymbol::Symbol symbol) {
  switch (symbol) {
    case theatre::FixtureSymbol::Hidden:
      return 0.0;
    case theatre::FixtureSymbol::Small:
      return 0.3;
    case theatre::FixtureSymbol::Normal:
      return 0.4;
    case theatre::FixtureSymbol::Large:
      return 0.5;
  }
  return 0.4;
}

constexpr double radiusSq(theatre::FixtureSymbol::Symbol symbol) {
  const double r = radius(symbol) * radius(symbol);
  return r;
}

double GetScale(const theatre::Management& management, double width, double height) {
  const theatre::Theatre &theatre = management.GetTheatre();
  theatre::Position extend = theatre.Extend();
  if (extend.X() == 0.0 || extend.Y() == 0.0)
    return 1.0;
  else
    return std::min(width / extend.X(), height / extend.Y());
}

double GetInvScale(const theatre::Management& management, double width, double height) {
  const double sc = GetScale(management, width, height);
  if (sc == 0.0)
    return 1.0;
  else
    return 1.0 / sc;
}
  
} // namespace

RenderEngine::RenderEngine(const theatre::Management& management) :
  management_(management)
{
}

void RenderEngine::DrawSnapshot(
  const Cairo::RefPtr<Cairo::Context> &cairo,
                    const theatre::ValueSnapshot &snapshot,
                    const DrawStyle &style,
                  const std::vector<theatre::Fixture *>& selectedFixtures) {
  const std::vector<std::unique_ptr<theatre::Fixture>> &fixtures =
      management_.GetTheatre().Fixtures();
  cairo->save();
  double sc = GetScale(management_, style.width, style.height);
  cairo->scale(sc, sc);

  state_.resize(fixtures.size());
  for (size_t fixtureIndex = 0; fixtureIndex != fixtures.size();
       ++fixtureIndex) {
    const theatre::Fixture &fixture = *fixtures[fixtureIndex];
    FixtureState &fixture_state = state_[fixtureIndex];
    if (fixture.IsVisible()) {
      size_t shapeCount = fixture.Type().ShapeCount();
      for (size_t i = 0; i != shapeCount; ++i) {
        const size_t shapeIndex = shapeCount - i - 1;
        const theatre::Color c = fixture.GetColor(snapshot, shapeIndex);

        cairo->set_source_rgb((double)c.Red() / 224.0 + 0.125,
                              (double)c.Green() / 224.0 + 0.125,
                              (double)c.Blue() / 224.0 + 0.125);

        const double singleRadius = radius(fixture.Symbol().Value());
        const double radius =
            shapeCount == 1
                ? singleRadius
                : 0.33 + 0.07 * double(shapeIndex) / (shapeCount - 1);
        const double x = fixture.GetPosition().X() + 0.5 + style.xOffset / sc;
        const double y = fixture.GetPosition().Y() + 0.5 + style.yOffset / sc;
        cairo->arc(x, y, radius, 0.0,
                   2.0 * M_PI);
        cairo->fill();
        
        const double direction_1 = fixture.Direction() - fixture.BeamAngle()*0.5;
        const double direction_2 = fixture.Direction() + fixture.BeamAngle()*0.5;
        const double beam_start_radius = radius * 1.2;
        const double beam_end_radius = radius * 10;
        Cairo::RefPtr<Cairo::RadialGradient> gradient =
          Cairo::RadialGradient::create(x, y, beam_start_radius, x, y, beam_end_radius);
        gradient->add_color_stop_rgba(0.0, (double)c.Red() / 255.0,
                              (double)c.Green() / 255.0,
                              (double)c.Blue() / 255.0, 0.5);
        gradient->add_color_stop_rgba(1.0, (double)c.Red() / 255.0,
                              (double)c.Green() / 255.0,
                              (double)c.Blue() / 255.0, 0.0);
        cairo->set_source(gradient);   
        const double cos_1 = std::cos(direction_1);
        const double sin_1 = std::sin(direction_1);
        const double cos_2 = std::cos(direction_2);
        const double sin_2 = std::sin(direction_2);
        cairo->arc_negative(x, y, beam_start_radius, direction_2, direction_1);
        cairo->line_to(x + cos_1*beam_end_radius, y + sin_1*beam_end_radius);
        cairo->line_to(x + cos_2*beam_end_radius, y + sin_2*beam_end_radius);
        cairo->line_to(x + cos_2*beam_start_radius, y + sin_2*beam_start_radius);
        cairo->fill();

        const int rotation = fixture.GetRotationSpeed(snapshot, shapeIndex);
        if (rotation != 0) {
          const double rotationDisp =
              M_PI * double(rotation) * style.timeSince / (10.0 * (1 << 24));
          fixture_state.rotation = std::fmod(rotationDisp + fixture_state.rotation, M_PI);
          const double s = std::sin(fixture_state.rotation);
          const double c = std::cos(fixture_state.rotation);
          cairo->set_line_width(radius * 0.2);
          cairo->set_source_rgb(0, 0, 0);
          cairo->move_to(x + c * radius + style.xOffset / sc,
                         y + s * radius + style.yOffset / sc);
          cairo->line_to(x - c * radius + style.xOffset / sc,
                         y - s * radius + style.yOffset / sc);
          cairo->move_to(x + s * radius + style.xOffset / sc,
                         y - c * radius + style.yOffset / sc);
          cairo->line_to(x - s * radius + style.xOffset / sc,
                         y + c * radius + style.yOffset / sc);
          cairo->stroke();
        }
      }
    }
  }
  cairo->set_source_rgb(0.2, 0.2, 1.0);
  cairo->set_line_width(4.0 / sc);
  for (const theatre::Fixture *f : selectedFixtures) {
    if (f->IsVisible()) {
      double rad = radius(f->Symbol().Value());
      double x = f->GetPosition().X() + 0.5;
      double y = f->GetPosition().Y() + 0.5;
      cairo->arc(x + style.xOffset / sc, y + style.yOffset / sc, rad, 0.0,
                 2.0 * M_PI);
      cairo->stroke();
    }
  }

  cairo->restore();
}

theatre::Fixture *RenderEngine::FixtureAt(const theatre::Position &position) const {
  const std::vector<std::unique_ptr<theatre::Fixture>> &fixtures =
      management_.GetTheatre().Fixtures();

  theatre::Fixture *fixture = nullptr;
  double closest = std::numeric_limits<double>::max();
  for (const std::unique_ptr<theatre::Fixture> &f : fixtures) {
    if (f->IsVisible() &&
        position.InsideRectangle(f->GetPosition(),
                                 f->GetPosition().Add(1.0, 1.0))) {
      double distanceSq =
          position.SquaredDistance(f->GetPosition().Add(0.5, 0.5));
      double radSq = radiusSq(f->Symbol().Value());
      if (distanceSq <= radSq && distanceSq < closest) {
        fixture = f.get();
        closest = distanceSq;
      }
    }
  }
  return fixture;
}

theatre::Position RenderEngine::MouseToPosition(double mouse_x, double mouse_y, double width, double height) const {
  const double sc = GetInvScale(management_, width, height);
  return theatre::Position(mouse_x, mouse_y) * sc;
}

}

