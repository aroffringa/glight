#ifndef GUI_FADER_WIDGET_H_
#define GUI_FADER_WIDGET_H_

#include "controlwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/overlay.h>
#include <gtkmm/scale.h>

#include "../components/iconbutton.h"

#include "../../theatre/forwards.h"

namespace glight::gui {

/**
 * @author Andre Offringa
 */
class FaderWidget final : public ControlWidget {
 public:
  FaderWidget(theatre::Management &management, EventTransmitter &eventHub,
              ControlMode mode, char key);

  void Toggle() override;
  void FullOn() override;
  void FullOff() override;
  void OnAssigned(bool moveFader) override;
  void MoveSlider() override;

  void Limit(double value) override {
    if (_scale.get_value() > value) _scale.set_value(value);
  }

  Gtk::Widget &NameLabel() { return _labelEventBox; }

 private:
  void ShowFadeButtons(bool mouse_in);
  void onScaleChange();
  void onOnButtonClicked();
  void ShowAssignDialog();
  bool onFlashButtonPressed(GdkEventButton *event);
  bool onFlashButtonReleased(GdkEventButton *event);
  void onFadeUp();
  void onFadeDown();
  bool HandleRightPress(GdkEventButton *event);
  bool HandleRightRelease(GdkEventButton *event);
  void MakePopupMenu();

  Gtk::EventBox _mouseInBox;
  Gtk::Overlay _overlay;
  Gtk::VBox _box;
  Gtk::Button _fadeUpButton;
  Gtk::VScale _scale;
  Gtk::Button _fadeDownButton;
  Gtk::Button _flashButton;
  IconButton _checkButton;
  Gtk::EventBox _labelEventBox;
  Gtk::Label _nameLabel;

  // right click menu
  Gtk::Menu _menu;
  Gtk::MenuItem _miAssign{"Assign..."};
  Gtk::SeparatorMenuItem _miSeperator1;
  Gtk::CheckMenuItem _miDisplayLabel{"Display label"};
  Gtk::CheckMenuItem _miDisplayFlashButton{"Display flash button"};
  Gtk::CheckMenuItem _miDisplayCheckButton{"Display check button"};
  Gtk::CheckMenuItem _miOverlayFadeButtons{"Overlay fade buttons"};

  bool _mouseIn = false;
  bool _holdUpdates = false;
};

}  // namespace glight::gui

#endif
