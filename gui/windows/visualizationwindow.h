#ifndef GUI_VISUALIZATIONWINDOW_H_
#define GUI_VISUALIZATIONWINDOW_H_

#include "../../theatre/fixturesymbol.h"
#include "../../theatre/forwards.h"
#include "../../theatre/position.h"

#include <gdkmm/pixbuf.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/menu.h>
#include <gtkmm/radiomenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/window.h>

namespace glight::gui {

class EventTransmitter;
class FixtureSelection;
class ShowWindow;

class VisualizationWindow : public Gtk::Window {
 public:
  VisualizationWindow(theatre::Management *management,
                      EventTransmitter *eventTransmitter,
                      FixtureSelection *fixtureSelection,
                      ShowWindow *showWindow);
  ~VisualizationWindow();

  void Update() { queue_draw(); }

  void SetDryMode(theatre::Management *dryManagement) {
    _dryManagement = dryManagement;
    Update();
  }
  void SetRealMode() {
    _dryManagement = nullptr;
    Update();
  }
  void MakeDryModeReal() {
    _management = _dryManagement;
    _dryManagement = nullptr;
    Update();
  }

 private:
  Gtk::DrawingArea _drawingArea;
  theatre::Management *_management;
  theatre::Management *_dryManagement;
  class EventTransmitter *_eventTransmitter;
  class FixtureSelection *_globalSelection;
  sigc::connection _globalSelectionConnection;
  class ShowWindow *_showWindow;
  bool _isInitialized, _isTimerRunning;
  sigc::connection _timeoutConnection;
  enum DragType {
    NotDragging,
    DragFixture,
    DragRectangle,
    DragAddRectangle
  } _dragType;
  std::vector<class Fixture *> _selectedFixtures, _selectedFixturesBeforeDrag;
  theatre::Position _draggingStart, _draggingTo;
  struct FixtureState {
    double rotation = 0.0;
  };
  std::vector<FixtureState> _fixtureStates;

  Gtk::Menu _popupMenu;
  Gtk::SeparatorMenuItem _miSeparator1, _miSeparator2;
  Gtk::MenuItem _miSymbolMenu, _miDryModeStyle, _miAlignHorizontally,
      _miAlignVertically, _miDistributeEvenly, _miAdd, _miRemove, _miDesign;
  Gtk::CheckMenuItem _miFullscreen;
  Gtk::Menu _symbolMenu, _dryModeStyleMenu;
  std::vector<Gtk::MenuItem> _miSymbols;
  Gtk::RadioMenuItem _miDMSSingle, _miDMSVertical, _miDMSHorizontal,
      _miDMSShadow;

  void inializeContextMenu();
  void initialize();
  void drawAll(const Cairo::RefPtr<Cairo::Context> &cairo);
  struct DrawStyle {
    size_t xOffset;
    size_t yOffset;
    size_t width;
    size_t height;
    double timeSince;
  };
  void drawManagement(const Cairo::RefPtr<Cairo::Context> &cairo,
                      class Management &management, const DrawStyle &style);
  void onTheatreChanged();
  bool onButtonPress(GdkEventButton *event);
  bool onButtonRelease(GdkEventButton *event);
  bool onMotion(GdkEventMotion *event);
  bool onExpose(const Cairo::RefPtr<Cairo::Context> &context);
  bool onTimeout() {
    Update();
    return true;
  }
  static double radius(theatre::FixtureSymbol::Symbol symbol) {
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
  static double radiusSq(theatre::FixtureSymbol::Symbol symbol) {
    return radius(symbol) * radius(symbol);
  }

  void onAlignHorizontally();
  void onAlignVertically();
  void onDistributeEvenly();
  void onAddFixtures();
  void onRemoveFixtures();
  void onDesignFixtures();
  void onFullscreen();
  void onSetSymbol(theatre::FixtureSymbol::Symbol symbol);
  void onGlobalSelectionChanged();

  double scale(Management &management, double width, double height);
  double invScale(Management &management, double width, double height) {
    double sc = scale(management, width, height);
    if (sc == 0.0)
      return 1.0;
    else
      return 1.0 / sc;
  }
  class Fixture *fixtureAt(theatre::Management &management,
                           const theatre::Position &position);
  void selectFixtures(const theatre::Position &a, const theatre::Position &b);
  void addFixtures(const theatre::Position &a, const theatre::Position &b);
};

}  // namespace glight::gui

#endif
