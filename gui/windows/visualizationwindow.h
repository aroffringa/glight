#ifndef VISUALIZATIONWINDOW_H
#define VISUALIZATIONWINDOW_H

#include "../../theatre/fixturesymbol.h"
#include "../../theatre/position.h"

#include <gdkmm/pixbuf.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/menu.h>
#include <gtkmm/radiomenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/window.h>

/**
        @author Andre Offringa
*/
class VisualizationWindow : public Gtk::Window {
 public:
  VisualizationWindow(class Management *management,
                      class EventTransmitter *eventTransmitter,
                      class FixtureSelection *fixtureSelection,
                      class ShowWindow *showWindow);
  ~VisualizationWindow();

  void Update() { queue_draw(); }

  void SetDryMode(class Management *dryManagement) {
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
  Management *_management;
  Management *_dryManagement;
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
  Position _draggingStart, _draggingTo;
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
  static double radius(FixtureSymbol::Symbol symbol) {
    switch (symbol) {
      case FixtureSymbol::Hidden:
        return 0.0;
      case FixtureSymbol::Small:
        return 0.3;
      case FixtureSymbol::Normal:
        return 0.4;
      case FixtureSymbol::Large:
        return 0.5;
    }
    return 0.4;
  }
  static double radiusSq(FixtureSymbol::Symbol symbol) {
    return radius(symbol) * radius(symbol);
  }

  void onAlignHorizontally();
  void onAlignVertically();
  void onDistributeEvenly();
  void onAddFixtures();
  void onRemoveFixtures();
  void onDesignFixtures();
  void onFullscreen();
  void onSetSymbol(FixtureSymbol::Symbol symbol);
  void onGlobalSelectionChanged();

  double scale(Management &management, double width, double height);
  double invScale(Management &management, double width, double height) {
    double sc = scale(management, width, height);
    if (sc == 0.0)
      return 1.0;
    else
      return 1.0 / sc;
  }
  class Fixture *fixtureAt(Management &management, const Position &position);
  void selectFixtures(const Position &a, const Position &b);
  void addFixtures(const Position &a, const Position &b);
};

#endif
