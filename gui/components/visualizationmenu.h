#ifndef GLIGHT_GUI_VISUALIZATION_MENU_H_
#define GLIGHT_GUI_VISUALIZATION_MENU_H_

#include <vector>

#include <giomm/actionmap.h>
#include <giomm/menu.h>
#include <giomm/menuitem.h>

#include <gtkmm/popovermenu.h>

#include "theatre/fixturesymbol.h"

namespace glight::theatre {
class Management;
}

namespace glight::gui {

enum class DryModeStyle { Primary, Secondary, Vertical, Horizontal, Shadow };

class VisualizationMenu : public Gtk::PopoverMenu {
 public:
  VisualizationMenu(Gio::ActionMap& map);

  sigc::signal<void()> SignalSetFullOn;
  sigc::signal<void()> SignalSetOff;
  sigc::signal<void()> SignalSetColor;
  sigc::signal<void()> SignalTrack;
  sigc::signal<void()> SignalTrackPan;

  sigc::signal<void(theatre::FixtureSymbol::Symbol)> SignalSelectSymbol;

  sigc::signal<void()> SignalDryStyleChange;

  sigc::signal<void()> SignalAlignHorizontally;
  sigc::signal<void()> SignalAlignVertically;
  sigc::signal<void()> SignalDistributeEvenly;
  sigc::signal<void()> SignalAddFixtures;
  sigc::signal<void()> SignalAddPreset;
  sigc::signal<void()> SignalRemoveFixtures;
  sigc::signal<void()> SignalGroupFixtures;
  sigc::signal<void()> SignalDesignFixtures;
  sigc::signal<void()> SignalFixtureProperties;
  sigc::signal<void()> SignalSaveImage;

  DryModeStyle GetDryModeStyle() const;

  void SetSensitivity(bool is_layout_locked, size_t n_selected);

 private:
  std::shared_ptr<Gio::SimpleAction> set_full_on_;
  std::shared_ptr<Gio::SimpleAction> set_off_;
  std::shared_ptr<Gio::SimpleAction> set_color_;
  std::shared_ptr<Gio::SimpleAction> set_track_;
  std::shared_ptr<Gio::SimpleAction> set_pan_track_;

  std::shared_ptr<Gio::SimpleAction> align_horizontally_;
  std::shared_ptr<Gio::SimpleAction> align_vertically_;
  std::shared_ptr<Gio::SimpleAction> distribute_evenly_;

  std::shared_ptr<Gio::SimpleAction> add_fixture_;
  std::shared_ptr<Gio::SimpleAction> add_preset_;
  std::shared_ptr<Gio::SimpleAction> remove_fixtures_;
  std::shared_ptr<Gio::SimpleAction> group_fixtures_;
  std::shared_ptr<Gio::SimpleAction> design_;

  std::shared_ptr<Gio::SimpleAction> properties_;
  std::shared_ptr<Gio::SimpleAction> save_image_;

  std::vector<std::shared_ptr<Gio::SimpleAction>> symbols_;
  DryModeStyle dry_mode_style_ = DryModeStyle::Primary;
};

}  // namespace glight::gui

#endif
