#ifndef GLIGHT_GUI_STATE_H_
#define GLIGHT_GUI_STATE_H_

#include <memory>
#include <vector>

#include <sigc++/signal.h>

#include "fadersetstate.h"

namespace glight::gui {

class GUIState {
 public:
  std::vector<std::unique_ptr<FaderSetState>> &FaderSets() {
    return fader_sets_;
  }
  const std::vector<std::unique_ptr<FaderSetState>> &FaderSets() const {
    return fader_sets_;
  }

  /**
   * Emitted when fader sets are added or removed from the list. This
   * is not emitted when individual faders are changed; it is only for
   * situations in which the list of fader sets are shown, e.g. as for
   * the menu items in the main window.
   */
  sigc::signal<void()> &FaderSetSignalChange() {
    return fader_set_signal_change_;
  }

  void EmitFaderSetChangeSignal() { fader_set_signal_change_(); }

  void Clear() {
    fader_sets_.clear();
    layout_locked_ = false;
    show_fixtures = true;
    show_beams = true;
    show_projections = true;
    show_crosshairs = true;
    show_stage_borders = true;
  }

  bool Empty() const { return fader_sets_.empty(); }

  bool IsAssigned(const theatre::SourceValue *s) const {
    for (const std::unique_ptr<FaderSetState> &fader : fader_sets_) {
      if (fader->IsAssigned(s)) return true;
    }
    return false;
  }

  bool LayoutLocked() const { return layout_locked_; }
  void SetLayoutLocked(bool layout_locked) { layout_locked_ = layout_locked; }

  /**
   * Returns the first unassigned controller. It only considers the types fader
   * or toggle buttons.
   */
  FaderState *GetFirstUnassignedFader() {
    for (std::unique_ptr<FaderSetState> &set : fader_sets_) {
      FaderState *fader = set->GetFirstUnassigned();
      if (fader) return fader;
    }
    return nullptr;
  }

  void SetWindowDimensions(size_t width, size_t height) {
    window_width = width;
    window_height = height;
  }
  size_t WindowWidth() const { return window_width; }
  size_t WindowHeight() const { return window_height; }
  void SetTheatreDimensions(double width, double depth, double height) {
    theatre_width = width;
    theatre_depth = depth;
    theatre_height = height;
  }
  double TheatreWidth() const { return theatre_width; }
  double TheatreDepth() const { return theatre_depth; }
  double TheatreHeight() const { return theatre_height; }
  void SetFixtureSize(double size) { fixture_size = size; }

  /**
   * The size (diameter) with which fixtures are displayed in
   * the schematic view, in meters.
   */
  double FixtureSize() const { return fixture_size; }

  bool ShowFixtures() const { return show_fixtures; }
  void SetShowFixtures(bool _show_fixtures) { show_fixtures = _show_fixtures; }
  bool ShowBeams() const { return show_beams; }
  void SetShowBeams(bool _show_beams) { show_beams = _show_beams; }
  bool ShowProjections() const { return show_projections; }
  void SetShowProjections(bool _show_projections) {
    show_projections = _show_projections;
  }
  bool ShowCrosshairs() const { return show_crosshairs; }
  void SetShowCrosshairs(bool _show_crosshairs) {
    show_crosshairs = _show_crosshairs;
  }
  bool ShowStageBorders() const { return show_stage_borders; }
  void SetShowStageBorders(bool show_borders) {
    show_stage_borders = show_borders;
  }

 private:
  bool layout_locked_ = false;
  size_t window_width = 0;
  size_t window_height = 0;
  double theatre_width = 0;
  double theatre_depth = 0;
  double theatre_height = 0;
  double fixture_size = 0;
  bool show_fixtures = true;
  bool show_beams = true;
  bool show_projections = true;
  bool show_crosshairs = true;
  bool show_stage_borders = true;
  sigc::signal<void()> fader_set_signal_change_;
  std::vector<std::unique_ptr<FaderSetState>> fader_sets_;
};

}  // namespace glight::gui

#endif
