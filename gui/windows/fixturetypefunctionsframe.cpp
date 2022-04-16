#include "fixturetypefunctionsframe.h"

FixtureTypeFunctionsFrame::FixtureTypeFunctionsFrame()
    : Gtk::Frame("Functions"),
      add_function_button_("+"),
      remove_function_button_("-") {
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

void FixtureTypeFunctionsFrame::onSelectionChanged() {}
