#ifndef GUI_WINDOWS_FIXTURE_TYPE_FUNCTIONS_FRAME_H_
#define GUI_WINDOWS_FIXTURE_TYPE_FUNCTIONS_FRAME_H_

#include "../../theatre/fixturetype.h"

#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/grid.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

namespace glight::gui {

class FixtureTypeFunctionsFrame : public Gtk::Frame {
 public:
  FixtureTypeFunctionsFrame();
  std::vector<theatre::FixtureTypeFunction> GetFunctions() const;
  void SetFunctions(const std::vector<theatre::FixtureTypeFunction> &functions);

 private:
  void onAdd();
  void onRemove();
  void onSelectionChanged();

  Gtk::ScrolledWindow functions_scrollbars_;
  Gtk::TreeView functions_view_;
  Glib::RefPtr<Gtk::ListStore> functions_model_;
  struct FunctionsColumns : public Gtk::TreeModelColumnRecord {
    FunctionsColumns() {
      add(dmx_offset_);
      add(is_16_bit_);
      add(function_type_);
      add(function_type_str_);
    }

    Gtk::TreeModelColumn<size_t> dmx_offset_;
    Gtk::TreeModelColumn<bool> is_16_bit_;
    Gtk::TreeModelColumn<theatre::FunctionType> function_type_;
    Gtk::TreeModelColumn<Glib::ustring> function_type_str_;
  } functions_columns_;

  Gtk::Grid grid_;
  Gtk::HBox functions_button_box_;
  Gtk::Button add_function_button_;
  Gtk::Button remove_function_button_;

  Gtk::Label dmx_offset_label_;
  Gtk::Entry dmx_offset_entry_;
  Gtk::CheckButton is_16_bit_button_;
  Gtk::Label function_type_label_;
  Glib::RefPtr<Gtk::ListStore> function_type_model_;
  Gtk::ComboBox function_type_combo_;

  struct FunctionTypeColumns : public Gtk::TreeModelColumnRecord {
    FunctionTypeColumns() {
      add(function_type_str_);
      add(function_type_);
    }
    Gtk::TreeModelColumn<theatre::FunctionType> function_type_;
    Gtk::TreeModelColumn<Glib::ustring> function_type_str_;
  } function_type_columns_;
};

}  // namespace glight::gui

#endif
