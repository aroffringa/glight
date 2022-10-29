#ifndef GUI_ICON_BUTTON_H_
#define GUI_ICON_BUTTON_H_

#include <sigc++/signal.h>

#include <gtkmm/drawingarea.h>

#include "../../theatre/color.h"

namespace glight::gui {
  
class IconButton : public Gtk::DrawingArea
{
public:
  IconButton();
  
  sigc::signal<void()>& SignalClicked() { return signal_clicked_; }
  bool GetActive() const { return active_; }
  void SetActive(bool active) {
    if(active != active_) {
      active_ = active;
      Update();
    }
  }
  
  void SetColors(const std::vector<theatre::Color>& color) {
    if(color != colors_) {
      colors_ = color;
      Update();
    }
  }
  
private:
  void Draw(const Cairo::RefPtr<Cairo::Context> &cairo); 
  void Update() { queue_draw(); }
  bool OnPress(GdkEventButton* event);
  bool OnRelease(GdkEventButton* event);
  bool OnEnter(GdkEventCrossing* event);
  bool OnLeave(GdkEventCrossing* event);
  
  bool active_ = false;
  bool entered_ = false;
  sigc::signal<void()> signal_clicked_;
  std::vector<theatre::Color> colors_;
};

}

#endif
