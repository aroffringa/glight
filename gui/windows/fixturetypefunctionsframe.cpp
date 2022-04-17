#include "fixturetypefunctionsframe.h"

FixtureTypeFunctionsFrame::FixtureTypeFunctionsFrame()
    : Gtk::Frame("Functions"),
      add_function_button_("+"),
      remove_function_button_("-"),
      dmx_offset_label_("DMX offset:"),
      is_16_bit_button_("16 bit"),
      function_type_label_("Function type:") {
  functions_model_ = Gtk::ListStore::create(functions_columns_);

  functions_view_.set_model(functions_model_);
  functions_view_.append_column("DMX", functions_columns_.dmx_offset_);
  functions_view_.append_column("16 bit?", functions_columns_.is_16_bit_);
  functions_view_.append_column("Type", functions_columns_.function_type_str_);
  functions_view_.set_vexpand(true);
  functions_view_.set_hexpand(true);
  functions_view_.get_selection()->signal_changed().connect(
      [&]() { onSelectionChanged(); });
  grid_.attach(functions_view_, 0, 0, 2, 1);

  add_function_button_.signal_clicked().connect([&]() { onAdd(); });
  functions_button_box_.pack_start(add_function_button_);
  remove_function_button_.signal_clicked().connect([&]() { onRemove(); });
  functions_button_box_.pack_end(remove_function_button_);
  grid_.attach(functions_button_box_, 0, 1, 2, 1);
  grid_.set_hexpand(true);
  grid_.set_vexpand(true);

  grid_.attach(dmx_offset_label_, 0, 2);
  dmx_offset_entry_.signal_changed().connect([&]() {
    Gtk::TreeModel::iterator selected =
        functions_view_.get_selection()->get_selected();
    if (selected) {
      const int val = std::atoi(dmx_offset_entry_.get_text().c_str());
      (*selected)[functions_columns_.dmx_offset_] = std::clamp(val, 0, 511);
    }
  });
  grid_.attach(dmx_offset_entry_, 1, 2);
  is_16_bit_button_.signal_clicked().connect([&]() {
    Gtk::TreeModel::iterator selected =
        functions_view_.get_selection()->get_selected();
    if (selected) {
      (*selected)[functions_columns_.is_16_bit_] =
          is_16_bit_button_.get_active();
    }
  });
  grid_.attach(is_16_bit_button_, 1, 3);
  grid_.attach(function_type_label_, 0, 4);

  function_type_model_ = Gtk::ListStore::create(function_type_columns_);
  const std::vector<FunctionType> types = GetFunctionTypes();
  for (FunctionType t : types) {
    Gtk::TreeModel::iterator iter = function_type_model_->append();
    (*iter)[function_type_columns_.function_type_] = t;
    (*iter)[function_type_columns_.function_type_str_] =
        FunctionTypeDescription(t);
  }
  function_type_combo_.set_model(function_type_model_);
  function_type_combo_.pack_start(function_type_columns_.function_type_str_);
  function_type_combo_.signal_changed().connect([&]() {
    Gtk::TreeModel::iterator selected =
        functions_view_.get_selection()->get_selected();
    if (selected) {
      Gtk::TreeModel::const_iterator iter = function_type_combo_.get_active();
      if (iter) {
        (*selected)[functions_columns_.function_type_] =
            FunctionType((*iter)[function_type_columns_.function_type_]);
        (*selected)[functions_columns_.function_type_str_] =
            Glib::ustring((*iter)[function_type_columns_.function_type_str_]);
      }
    }
  });
  grid_.attach(function_type_combo_, 1, 4);

  add(grid_);
}

std::vector<FixtureTypeFunction> FixtureTypeFunctionsFrame::GetFunctions()
    const {
  std::vector<FixtureTypeFunction> functions;
  for (const Gtk::TreeRow &child : functions_model_->children()) {
    const size_t dmx_offset = child[functions_columns_.dmx_offset_];
    const FunctionType type = child[functions_columns_.function_type_];
    const bool is_16_bit = child[functions_columns_.is_16_bit_];
    functions.emplace_back(dmx_offset, type, is_16_bit);
  }
  return functions;
}

void FixtureTypeFunctionsFrame::SetFunctions(
    const std::vector<FixtureTypeFunction> &functions) {
  functions_model_->clear();
  for (size_t i = 0; i != functions.size(); ++i) {
    Gtk::TreeModel::iterator iter = functions_model_->append();
    Gtk::TreeModel::Row row = *iter;
    const FixtureTypeFunction &f = functions[i];
    row[functions_columns_.dmx_offset_] = f.dmxOffset;
    row[functions_columns_.is_16_bit_] = f.is16Bit;
    row[functions_columns_.function_type_] = f.type;
    row[functions_columns_.function_type_str_] =
        FunctionTypeDescription(f.type);
  }
}

void FixtureTypeFunctionsFrame::onAdd() {
  size_t dmx_offset = 0;
  if (!functions_model_->children().empty()) {
    Gtk::TreeModel::Row row = *functions_model_->children().rbegin();
    if (row[functions_columns_.is_16_bit_])
      dmx_offset = row[functions_columns_.dmx_offset_] + 2;
    else
      dmx_offset = row[functions_columns_.dmx_offset_] + 1;
  }

  Gtk::TreeModel::iterator iter = functions_model_->append();
  Gtk::TreeModel::Row row = *iter;
  row[functions_columns_.dmx_offset_] = dmx_offset;
  row[functions_columns_.is_16_bit_] = false;
  row[functions_columns_.function_type_] = FunctionType::White;
  row[functions_columns_.function_type_str_] =
      FunctionTypeDescription(FunctionType::White);
}

void FixtureTypeFunctionsFrame::onRemove() {
  Glib::RefPtr<Gtk::TreeSelection> selection = functions_view_.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected) {
    functions_model_->erase(selected);
  }
}

void FixtureTypeFunctionsFrame::onSelectionChanged() {
  Gtk::TreeModel::iterator selected =
      functions_view_.get_selection()->get_selected();
  dmx_offset_entry_.set_sensitive(selected);
  is_16_bit_button_.set_sensitive(selected);
  function_type_combo_.set_sensitive(selected);
  if (selected) {
    dmx_offset_entry_.set_text(
        std::to_string((*selected)[functions_columns_.dmx_offset_]));
    is_16_bit_button_.set_active((*selected)[functions_columns_.is_16_bit_]);
    const int ft_index = static_cast<int>(
        FunctionType((*selected)[functions_columns_.function_type_]));
    function_type_combo_.set_active(ft_index);
  } else {
    dmx_offset_entry_.set_text("");
    is_16_bit_button_.set_active(false);
    function_type_combo_.set_active(-1);
  }
}
