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

  template <typename NObject>
  void SetList(const std::vector<NObject *> &objects);

  void Append(theatre::NamedObject &object);

  std::vector<theatre::NamedObject *> GetList() const;

  sigc::signal<void()> &SignalChanged() { return signal_changed_; }

  void SetShowTypeColumn(bool show_type_column) {
    show_type_column_ = show_type_column;
  }

 private:
  void FillList();
  void MoveUp();
  void MoveDown();
  void Remove();
  void SetColumns();

  theatre::Management &management_;
  EventTransmitter &hub_;

  Glib::RefPtr<Gtk::ListStore> model_;
  Gtk::TreeView view_;
  struct Columns : public Gtk::TreeModelColumnRecord {
    Columns() {
      add(title_);
      add(type_);
      add(object_);
    }

    Gtk::TreeModelColumn<Glib::ustring> title_;
    Gtk::TreeModelColumn<Glib::ustring> type_;
    Gtk::TreeModelColumn<theatre::NamedObject *> object_;
  } columns_;
  Gtk::ScrolledWindow scrolled_window_;

  Gtk::VBox button_box_;
  Gtk::Button up_button_;
  Gtk::Button down_button_;
  Gtk::Button remove_button_;
  sigc::signal<void()> signal_changed_;
  bool show_type_column_ = true;
};

template <typename NObject>
void ReorderWidget::SetList(const std::vector<NObject *> &objects) {
  model_->clear();

  for (theatre::NamedObject *object : objects) {
    Append(*object);
  }
  signal_changed_();
}

}  // namespace glight::gui::components

#endif
