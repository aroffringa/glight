#ifndef GLIGHT_GUI_WINDOWS_EDIT_COLOR_RANGE_H_
#define GLIGHT_GUI_WINDOWS_EDIT_COLOR_RANGE_H_

#include <vector>

#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/toolbutton.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "gui/recursionlock.h"
#include "gui/components/colorselectwidget.h"

#include "theatre/fixturefunctionparameters.h"

namespace glight::gui::windows {

class EditColorRange : public Gtk::Window {
 public:
  EditColorRange(std::vector<theatre::ColorRangeParameters::Range> ranges);

  std::vector<theatre::ColorRangeParameters::Range> GetRanges() const;

 private:
  void FillList();
  void UpdateList();
  void Add();
  void Remove();
  void OnSelectionChanged();
  void SaveChange();
  void SetSensitive(bool sensitive) {
    remove_button_.set_sensitive(sensitive);
    start_label_.set_sensitive(sensitive);
    start_entry_.set_sensitive(sensitive);
    end_label_.set_sensitive(sensitive);
    end_entry_.set_sensitive(sensitive);
    color_check_button_.set_sensitive(sensitive);
    color_selection_.set_sensitive(sensitive);
  }

  Gtk::Grid grid_;
  Gtk::Toolbar tool_bar_;
  Gtk::ToolButton add_button_{"+"};
  Gtk::ToolButton remove_button_{"-"};
  Gtk::TreeView list_view_;
  Glib::RefPtr<Gtk::ListStore> list_model_;
  struct ListColumns : public Gtk::TreeModelColumnRecord {
    ListColumns() {
      add(index_);
      add(start_);
      add(end_);
      add(color_);
    }

    Gtk::TreeModelColumn<unsigned> index_;
    Gtk::TreeModelColumn<unsigned> start_;
    Gtk::TreeModelColumn<unsigned> end_;
    Gtk::TreeModelColumn<Glib::ustring> color_;
  } list_columns_;
  Gtk::ScrolledWindow scrolled_window_;
  Gtk::Label start_label_{"Start:"};
  Gtk::Entry start_entry_;
  Gtk::Label end_label_{"End:"};
  Gtk::Entry end_entry_;
  Gtk::CheckButton color_check_button_{"Color:"};
  ColorSelectWidget color_selection_;
  RecursionLock lock_;

  std::vector<theatre::ColorRangeParameters::Range> ranges_;
};

}  // namespace glight::gui::windows

#endif
