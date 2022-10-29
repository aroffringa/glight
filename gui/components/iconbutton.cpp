#include "iconbutton.h"

namespace glight::gui {
namespace {
void DrawCircle(const Cairo::RefPtr<Cairo::Context>& cairo, const theatre::Color& color, bool entered, size_t index, size_t n) {
  if(entered) {
    cairo->set_source_rgb(color.RedRatio()/3.0+0.666, color.GreenRatio()/3.0+0.666, color.BlueRatio()/3.0+0.666);
  }
  else {
    cairo->set_source_rgb(color.RedRatio(), color.GreenRatio(), color.BlueRatio());
  }
  cairo->arc(0.0, 0.0, 1.0, 2.0*M_PI/n*index+M_PI_2, 2.0*M_PI/n*(index+1)+M_PI_2);
  cairo->fill();
}

void DrawCircleBorder(const Cairo::RefPtr<Cairo::Context>& cairo, bool entered) {
  cairo->arc(0.0, 0.0, 1.0, 0, 2.0*M_PI);
  if(entered) {
    cairo->set_source_rgb(0.5, 0.5, 0.5);
  }
  else {
    cairo->set_source_rgb(0.2, 0.2, 0.2);
  }
  cairo->stroke();
}

void DrawCross(const Cairo::RefPtr<Cairo::Context>& cairo) {
  cairo->set_source_rgb(0.2, 0.2, 0.2);
  constexpr double edge = M_SQRT1_2;
  cairo->move_to(edge, edge);
  cairo->line_to(-edge, -edge);
  cairo->move_to(edge, -edge);
  cairo->line_to(-edge, edge);
  cairo->stroke();
}  
}
  
IconButton::IconButton()
{
  set_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK |
                          Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
  signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context> &cairo) { Draw(cairo); return true; });
  signal_button_press_event().connect(
      sigc::mem_fun(*this, &IconButton::OnPress));
  signal_button_release_event().connect(
      sigc::mem_fun(*this, &IconButton::OnRelease));
  signal_enter_notify_event().connect(
      sigc::mem_fun(*this, &IconButton::OnEnter));
  signal_leave_notify_event().connect(
      sigc::mem_fun(*this, &IconButton::OnLeave));
  set_size_request(24, 24);
}

void IconButton::Draw(const Cairo::RefPtr<Cairo::Context>& cairo)
{
  const int width = get_width();
  const int height = get_height();
  const int size = std::min(width, height);
  const double radius = size*0.5 - std::min(5.0, size*0.1);
  if(radius > 0) {
    cairo->save();
    cairo->translate(width*0.5, height*0.5);
    const double scale = radius;
    cairo->scale(scale, scale);
    cairo->set_line_width(1.0/scale);
    if(colors_.empty()) {
      DrawCircle(cairo, theatre::Color::Gray(192), entered_, 0, 1);
    }
    else {
      const size_t n = colors_.size();
      for(size_t i=0; i!=n; ++i) {
        DrawCircle(cairo, colors_[i], entered_, i, n);
      }
    }
    DrawCircleBorder(cairo, entered_);
    if(active_) {
      DrawCross(cairo);
    }
    cairo->restore();
  }
}

bool IconButton::OnPress(GdkEventButton* event)
{
  return true;
}

bool IconButton::OnRelease(GdkEventButton* event)
{
  active_ = !active_;
  signal_clicked_();
  Update();
  return true;
}

bool IconButton::OnEnter(GdkEventCrossing* event)
{
  if(!entered_) {
    entered_ = true;
    Update();
  }
  return true;
}

bool IconButton::OnLeave(GdkEventCrossing* event)
{
  if(entered_) {
    entered_ = false;
    Update();
  }
  return true;
}

}
