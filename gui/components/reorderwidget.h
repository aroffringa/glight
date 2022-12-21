#ifndef GUI_COMPONENTS_REORDER_WIDGET_H_
#define GUI_COMPONENTS_REORDER_WIDGET_H_

#include <gtkmm/box.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>

#include "../../theatre/forwards.h"
#include "../../theatre/namedobject.h"

namespace glight::gui {
class EventTransmitter;
}

namespace glight::gui::components {

class ReorderWidget : public Gtk::HBox {
 public:
  ReorderWidget(theatre::Management &management, EventTransmitter &hub);

  void SetList(const std::vector<theatre::NamedObject *> &objects);

  std::vector<theatre::NamedObject *> GetList() const;

 private:
  void FillList();
  void MoveUp();
  void MoveDown();
  void Remove();

  theatre::Management &management_;
  EventTransmitter &hub_;

  Glib::RefPtr<Gtk::ListStore> model_;
  Gtk::TreeView view_;
  struct Columns : public Gtk::TreeModelColumnRecord {
    Columns() {
      add(title_);
      add(object_);
    }

    Gtk::TreeModelColumn<Glib::ustring> title_;
    Gtk::TreeModelColumn<theatre::NamedObject *> object_;
  } columns_;
  Gtk::ScrolledWindow scrolled_window_;

  Gtk::VBox button_box_;
  Gtk::Button up_button_;
  Gtk::Button down_button_;
  Gtk::Button remove_button_;
};

}  // namespace glight::gui::components

#endif
