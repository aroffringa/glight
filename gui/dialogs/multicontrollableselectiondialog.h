#ifndef GUI_DIALOG_MULTI_CONTROLLABLE_SELECTION_DIALOG_H_
#define GUI_DIALOG_MULTI_CONTROLLABLE_SELECTION_DIALOG_H_

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/grid.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "gui/recursionlock.h"
#include <sigc++/scoped_connection.h>

#include "gui/components/objectbrowser.h"

#include "theatre/controllable.h"

namespace glight::gui {

class EventTransmitter;

class MultiControllableSelectionDialog : public Gtk::Dialog {
 public:
  MultiControllableSelectionDialog();

  bool IsSelected(const theatre::Controllable& controllable) const;
  std::vector<theatre::Controllable*> GetSelection() const;
  void SetSelection(const std::vector<theatre::Controllable*>& selection);

 private:
  void OnObjectSelectionChanged();
  void OnViewSelectionChanged();
  void OnAdd();
  void OnRemove();
  void OnUpdateControllables();

  Gtk::Box box_;
  ObjectBrowser object_browser_;

  Gtk::Box button_box_{Gtk::Orientation::VERTICAL};
  Gtk::Button add_button_;
  Gtk::Button remove_button_;

  Gtk::Grid grid_;
  Gtk::TreeView selection_view_;
  Glib::RefPtr<Gtk::ListStore> selection_store_;
  struct SelectionColumns : public Gtk::TreeModelColumnRecord {
    SelectionColumns() {
      add(name_);
      add(controllable_);
    }

    Gtk::TreeModelColumn<Glib::ustring> name_;
    Gtk::TreeModelColumn<theatre::Controllable*> controllable_;
  } selection_columns_;
  Gtk::ScrolledWindow selection_scrolled_window_;

  RecursionLock recursion_lock_;
  sigc::scoped_connection update_connection_;
};

}  // namespace glight::gui

#endif
