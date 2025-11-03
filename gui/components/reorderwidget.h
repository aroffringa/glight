#ifndef GUI_COMPONENTS_REORDER_WIDGET_H_
#define GUI_COMPONENTS_REORDER_WIDGET_H_

#include <gtkmm/box.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>

#include "../../theatre/forwards.h"
#include "../../theatre/namedobject.h"

#include "system/trackableptr.h"

namespace glight::gui {
class EventTransmitter;
}

namespace glight::gui::components {

class ReorderWidget : public Gtk::Box {
 public:
  ReorderWidget();

  template <typename NObject>
  void SetList(const std::vector<system::ObservingPtr<NObject>> &objects);

  void Append(system::ObservingPtr<theatre::NamedObject> object);

  std::vector<system::ObservingPtr<theatre::NamedObject>> GetList() const;

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
    Gtk::TreeModelColumn<system::ObservingPtr<theatre::NamedObject>> object_;
  } columns_;
  Gtk::ScrolledWindow scrolled_window_;

  Gtk::Box button_box_{Gtk::Orientation::VERTICAL};
  Gtk::Button up_button_;
  Gtk::Button down_button_;
  Gtk::Button remove_button_;
  sigc::signal<void()> signal_changed_;
  bool show_type_column_ = true;
};

template <typename NObject>
void ReorderWidget::SetList(
    const std::vector<system::ObservingPtr<NObject>> &objects) {
  model_->clear();

  for (system::ObservingPtr<theatre::NamedObject> object : objects) {
    Append(object);
  }
  signal_changed_();
}

}  // namespace glight::gui::components

#endif
