#ifndef GUI_MAIN_MENU_H_
#define GUI_MAIN_MENU_H_

#include <memory>
#include <vector>

#include <giomm/simpleaction.h>
#include <gtkmm/popovermenubar.h>

#include "theatre/effecttype.h"

namespace glight::gui {

class FaderSetState;

class MainMenu : public Gtk::PopoverMenuBar {
 public:
  MainMenu();

  // File menu
  sigc::signal<void()> New;
  sigc::signal<void()> Open;
  sigc::signal<void()> Save;
  sigc::signal<void()> Import;
  sigc::signal<void()> Settings;
  sigc::signal<void()> Quit;

  // Design menu
  sigc::signal<void()> LockLayout;
  sigc::signal<void()> BlackOut;

  sigc::signal<void()> AddEmptyPreset;
  sigc::signal<void()> AddCurrentPreset;
  sigc::signal<void()> AddChase;
  sigc::signal<void()> AddTimeSequence;
  sigc::signal<void(theatre::EffectType)> AddEffect;
  sigc::signal<void()> AddFolder;
  sigc::signal<void()> DeleteObject;

  sigc::signal<void()> DesignWizard;
  sigc::signal<void()> TheatreDimensions;

  // View menu
  sigc::signal<void()> ShowFixtures;
  sigc::signal<void()> ShowBeams;
  sigc::signal<void()> ShowProjections;
  sigc::signal<void()> ShowStageBorders;
  sigc::signal<void()> FullScreen;

  // Window menu
  sigc::signal<void()> NewFaderWindow;
  sigc::signal<void()> FixtureList;
  sigc::signal<void()> FixtureTypes;
  sigc::signal<void()> SideBar;
  sigc::signal<void()> PowerMonitor;
  sigc::signal<void(bool active)> SceneWindow;
  sigc::signal<void(FaderSetState& fader_set)> FaderWindow;

  bool ShowFixturesActive() const { return GetState(show_fixtures_); }
  void SetShowFixtures(bool show_fixtures) {
    SetState(show_fixtures_, show_fixtures);
  }

  bool ShowBeamsActive() const { return GetState(show_beams_); }
  void SetShowBeams(bool show_beams) { SetState(show_beams_, show_beams); }

  bool ShowProjectionsActive() const { return GetState(show_projections_); }
  void SetShowProjections(bool show_projections) {
    SetState(show_projections_, show_projections);
  }

  bool ShowStageBordersActive() const { return GetState(show_stage_borders_); }
  void SetShowStageBorders(bool show_stage_borders) {
    SetState(show_stage_borders_, show_stage_borders);
  }

  bool FullScreenActive() const { return GetState(full_screen_); }

  bool FixtureListActive() const { return GetState(fixture_list_); }
  void SetFixtureListActive(bool active) { SetState(fixture_list_, active); }

  bool FixtureTypesActive() const { return GetState(fixture_types_); }
  void SetFixtureTypesActive(bool active) { SetState(fixture_types_, active); }

  bool SideBarActive() const { return GetState(side_bar_); }
  void SetSideBarActive(bool active) { SetState(side_bar_, active); }

  bool PowerMonitorActive() const { return GetState(power_monitor_); }
  void SetPowerMonitorActive(bool active) { SetState(power_monitor_, active); }

  void SetSceneWindowActive(bool active) { GetState(scene_window_); }

  void SetFaderList(const std::vector<std::unique_ptr<FaderSetState>>& faders);

  bool IsLayoutLocked() const { return GetState(layout_locked_); }
  void SetLayoutLocked(bool lock) { SetState(layout_locked_, lock); }

  void SetIsObjectSelected(bool is_selected) {
    delete_object_->set_enabled(is_selected);
  }

 private:
  bool GetState(const std::shared_ptr<Gio::SimpleAction>& action) const {
    bool value;
    action->get_state(value);
    return value;
  }
  void SetState(const std::shared_ptr<Gio::SimpleAction>& action, bool state) {
    action->set_state(Glib::Variant<bool>::create(state));
  }
  std::shared_ptr<Gio::SimpleAction> layout_locked_;
  std::shared_ptr<Gio::SimpleAction> delete_object_;

  std::shared_ptr<Gio::SimpleAction> show_fixtures_;
  std::shared_ptr<Gio::SimpleAction> show_beams_;
  std::shared_ptr<Gio::SimpleAction> show_projections_;
  std::shared_ptr<Gio::SimpleAction> show_stage_borders_;
  std::shared_ptr<Gio::SimpleAction> full_screen_;

  std::shared_ptr<Gio::SimpleAction> side_bar_;
  std::shared_ptr<Gio::SimpleAction> power_monitor_;
  std::shared_ptr<Gio::SimpleAction> fixture_list_;
  std::shared_ptr<Gio::SimpleAction> fixture_types_;

  std::shared_ptr<Gio::SimpleAction> scene_window_;
};

}  // namespace glight::gui

#endif
