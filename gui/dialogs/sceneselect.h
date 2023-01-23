#ifndef GUI_DIALOG_SCENE_SELLECT_H_
#define GUI_DIALOG_SCENE_SELLECT_H_

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/dialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "../../theatre/forwards.h"

#include <memory>

namespace glight::gui {
class EventTransmitter;
class FixtureSelection;
}  // namespace glight::gui

namespace glight::gui::dialogs {

/**
 * @author Andre Offringa
 */
class SceneSelect : public Gtk::Dialog {
 public:
  SceneSelect(theatre::Management &management, EventTransmitter &eventHub);
  ~SceneSelect();

  void SetSelection(theatre::Scene &scene);
  theatre::Scene *GetSelection();

 private:
  void FillScenesList();
  void OnSelectButtonClicked();
  void OnSelectionChanged();

  theatre::Management &management_;
  EventTransmitter &event_hub_;

  sigc::connection update_controllables_connection_;

  Gtk::TreeView view_;
  Glib::RefPtr<Gtk::ListStore> model_;
  struct Columns : public Gtk::TreeModelColumnRecord {
    Columns() {
      add(title_);
      add(items_);
      add(scene_);
    }

    Gtk::TreeModelColumn<Glib::ustring> title_;
    Gtk::TreeModelColumn<size_t> items_;
    Gtk::TreeModelColumn<theatre::Scene *> scene_;
  } columns_;
  Gtk::ScrolledWindow scrolled_window_;
  Gtk::Button *select_button_;
};

}  // namespace glight::gui::dialogs

#endif
