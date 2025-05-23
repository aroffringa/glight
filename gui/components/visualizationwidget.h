#ifndef GLIGHT_GUI_VISUALIZATION_WIDGET_H_
#define GLIGHT_GUI_VISUALIZATION_WIDGET_H_

#include <gdkmm/pixbuf.h>
#include <gtkmm/drawingarea.h>

#include "theatre/coordinate2d.h"
#include "theatre/fixturesymbol.h"
#include "theatre/forwards.h"
#include "theatre/valuesnapshot.h"

#include "gui/renderengine.h"
#include "gui/scopedconnection.h"

#include "system/deletableptr.h"

#include "visualizationmenu.h"

namespace glight::gui {

class EventTransmitter;
class FixtureSelection;
class MainWindow;

namespace windows {
class FixtureProperties;
}

enum class MouseState {
  Normal,
  DragFixture,
  DragRectangle,
  DragAddRectangle,
  Track,
  TrackPan,
  RotateFixture
};

class VisualizationWidget : public Gtk::DrawingArea {
 public:
  VisualizationWidget(theatre::Management *management,
                      EventTransmitter *eventTransmitter,
                      FixtureSelection *fixtureSelection,
                      MainWindow *showWindow);

  void Update() { queue_draw(); }

  void SetDrawFixtures(bool draw_fixtures) { draw_fixtures_ = draw_fixtures; }
  void SetDrawBeams(bool draw_beams) { draw_beams_ = draw_beams; }
  void SetDrawProjections(bool draw_projections) {
    draw_projections_ = draw_projections;
  }
  void SetDrawBorders(bool draw_borders) { draw_borders_ = draw_borders; }

 private:
  VisualizationWidget(const VisualizationWidget &) = delete;
  VisualizationWidget &operator=(const VisualizationWidget &) = delete;

  void inializeContextMenu();
  void initialize();
  void drawAll(const Cairo::RefPtr<Cairo::Context> &cairo);
  void DrawShapshot(
      const Cairo::RefPtr<Cairo::Context> &cairo,
      const std::vector<system::ObservingPtr<theatre::Fixture>> &selection,
      size_t width, size_t height);
  void updateMidiColors();
  void onTheatreChanged();
  bool onButtonPress(GdkEventButton *event);
  bool onButtonRelease(GdkEventButton *event);
  bool onMotion(GdkEventMotion *event);
  bool onExpose(const Cairo::RefPtr<Cairo::Context> &context);
  bool onTimeout();

  void onFixtureProperties();
  void onSaveImage();
  void onAlignHorizontally();
  void onAlignVertically();
  void onDistributeEvenly();
  void onAddFixtures();
  void onAddPreset();
  void onRemoveFixtures();
  void onGroupFixtures();
  void onDesignFixtures();
  void onSetSymbol(theatre::FixtureSymbol::Symbol symbol);
  void onGlobalSelectionChanged();

  void OnSetColor();
  void OnTrack();
  void OnTrackWithPan();

  void selectFixtures(const theatre::Coordinate2D &a,
                      const theatre::Coordinate2D &b);
  void addFixtures(const theatre::Coordinate2D &a,
                   const theatre::Coordinate2D &b);
  void SetTilt(const theatre::Coordinate2D &position);
  void SetPan(const theatre::Coordinate2D &position);
  void SetCursor(Gdk::CursorType cursor_type);

  bool draw_fixtures_ = true;
  bool draw_beams_ = true;
  bool draw_projections_ = true;
  bool draw_borders_ = true;

  theatre::Management *_management;
  EventTransmitter *_eventTransmitter;
  FixtureSelection *_globalSelection;
  ScopedConnection _globalSelectionConnection;
  MainWindow *main_window_;
  system::DeletablePtr<glight::gui::windows::FixtureProperties>
      _propertiesWindow;
  bool _isInitialized, _isTimerRunning;
  ScopedConnection _timeoutConnection;
  ScopedConnection update_connection_;
  MouseState _dragType;
  std::vector<system::ObservingPtr<theatre::Fixture>> _selectedFixtures;
  std::vector<system::ObservingPtr<theatre::Fixture>> _dragInvolvedFixtures;
  theatre::Coordinate2D _draggingStart;
  theatre::Coordinate2D _draggingTo;
  RenderEngine render_engine_;
  theatre::ValueSnapshot primary_snapshot_;
  theatre::ValueSnapshot secondary_snapshot_;
  Gdk::CursorType cursor_type_;
  std::unique_ptr<Gtk::Window> sub_window_;

  VisualizationMenu context_menu_;
};

}  // namespace glight::gui

#endif
