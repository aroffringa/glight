#ifndef GLIGHT_GUI_VISUALIZATION_MENU_H_
#define GLIGHT_GUI_VISUALIZATION_MENU_H_

#include <gtkmm/checkmenuitem.h>
#include <gtkmm/menu.h>
#include <gtkmm/radiomenuitem.h>
#include <gtkmm/separatormenuitem.h>

#include "theatre/fixturesymbol.h"

namespace glight::theatre {
class Management;
}

namespace glight::gui {

enum class DryModeStyle { Primary, Secondary, Vertical, Horizontal, Shadow };

class VisualizationMenu : public Gtk::Menu {
 public:
  VisualizationMenu();

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
  Gtk::SeparatorMenuItem _miSeparator1, _miSeparator2;
  Gtk::MenuItem mi_set_menu_{"Set"};
  Gtk::Menu set_menu_;
  Gtk::MenuItem mi_set_full_on_{"Full on"};
  Gtk::MenuItem mi_set_off_{"Off"};
  Gtk::MenuItem mi_set_color_{"Set color..."};
  Gtk::MenuItem mi_track_{"Track"};
  Gtk::MenuItem mi_track_pan_{"Track with pan"};

  Gtk::MenuItem _miSymbolMenu{"Symbol"};
  Gtk::Menu _symbolMenu;
  std::vector<Gtk::MenuItem> _miSymbols;

  Gtk::MenuItem _miDryModeStyle{"Dry mode style"};
  Gtk::Menu _dryModeStyleMenu;
  Gtk::RadioMenuItem _miDMSPrimary{"Primary"};
  Gtk::RadioMenuItem _miDMSSecondary{"Secondary"};
  Gtk::RadioMenuItem _miDMSVertical{"Vertical"};
  Gtk::RadioMenuItem _miDMSHorizontal{"Horizontal"};
  Gtk::RadioMenuItem _miDMSShadow{"Shadow"};

  Gtk::MenuItem _miAlignHorizontally{"Align horizontally"};
  Gtk::MenuItem _miAlignVertically{"Align vertically"};
  Gtk::MenuItem _miDistributeEvenly{"Distribute evenly"};
  Gtk::MenuItem _miAdd{"Add fixture..."};
  Gtk::MenuItem _miAddPreset{"Add preset"};
  Gtk::MenuItem _miRemove{"Remove"};
  Gtk::MenuItem _miGroup{"Group..."};
  Gtk::MenuItem _miDesign{"Design..."};
  Gtk::MenuItem _miProperties{"Properties"};
  Gtk::MenuItem _miSaveImage{"Save image..."};
};

}  // namespace glight::gui

#endif
