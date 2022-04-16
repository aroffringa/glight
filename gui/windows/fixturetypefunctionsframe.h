#ifndef GUI_WINDOWS_FIXTURE_TYPE_FUNCTIONS_FRAME_H_
#define GUI_WINDOWS_FIXTURE_TYPE_FUNCTIONS_FRAME_H_

#include "../../theatre/fixturetype.h"

#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

class FixtureTypeFunctionsFrame : public Gtk::Frame {
 public:
  FixtureTypeFunctionsFrame();
  std::vector<FixtureTypeFunction> GetFunctions() const;
  void SetFunctions(const std::vector<FixtureTypeFunction> &functions);

 private:
  void onAdd();
  void onRemove();
  void onSelectionChanged();

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
    Gtk::TreeModelColumn<FunctionType> function_type_;
    Gtk::TreeModelColumn<Glib::ustring> function_type_str_;
  } functions_columns_;

  Gtk::Grid grid_;
  Gtk::HBox functions_button_box_;
  Gtk::Button add_function_button_;
  Gtk::Button remove_function_button_;
};

#endif
