#include "fixturetypefunctionsframe.h"

namespace glight::gui {

using system::OptionalNumber;

namespace {
std::string FineToString(OptionalNumber<size_t> fine_channel) {
  if (fine_channel)
    return std::to_string(*fine_channel);
  else
    return "-";
}

OptionalNumber<size_t> GetFine(const std::string& str) {
  if (str == "" || str == "-")
    return {};
  else
    return OptionalNumber<size_t>(std::atoi(str.c_str()));
}

}  // namespace

FixtureTypeFunctionsFrame::FixtureTypeFunctionsFrame()
    : Gtk::Frame("Functions"),
      add_function_button_("+"),
      remove_function_button_("-"),
      dmx_offset_label_("DMX offset:"),
      fine_channel_label_("Fine channel:"),
      function_type_label_("Function type:") {
  functions_model_ = Gtk::ListStore::create(functions_columns_);

  functions_view_.set_model(functions_model_);
  functions_view_.append_column("DMX", functions_columns_.dmx_offset_);
  functions_view_.append_column("Fine", functions_columns_.fine_channel_);
  functions_view_.append_column("Type", functions_columns_.function_type_str_);
  functions_view_.set_vexpand(true);
  functions_view_.set_hexpand(true);
  functions_view_.get_selection()->signal_changed().connect(
      [&]() { onSelectionChanged(); });
  functions_scrollbars_.add(functions_view_);
  functions_scrollbars_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  grid_.attach(functions_scrollbars_, 0, 0, 2, 1);

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
  fine_channel_entry_.signal_changed().connect([&]() {
    Gtk::TreeModel::iterator selected =
        functions_view_.get_selection()->get_selected();
    if (selected) {
      (*selected)[functions_columns_.fine_channel_] =
          FineToString(GetFine(fine_channel_entry_.get_text()));
    }
  });
  grid_.attach(fine_channel_label_, 0, 3);
  grid_.attach(fine_channel_entry_, 1, 3);
  grid_.attach(function_type_label_, 0, 4);

  function_type_model_ = Gtk::ListStore::create(function_type_columns_);
  const std::vector<theatre::FunctionType> types = theatre::GetFunctionTypes();
  for (theatre::FunctionType t : types) {
    Gtk::TreeModel::iterator iter = function_type_model_->append();
    (*iter)[function_type_columns_.function_type_] = t;
    (*iter)[function_type_columns_.function_type_str_] = ToString(t);
  }
  function_type_combo_.set_model(function_type_model_);
  function_type_combo_.pack_start(function_type_columns_.function_type_str_);
  function_type_combo_.signal_changed().connect([&]() {
    Gtk::TreeModel::iterator selected =
        functions_view_.get_selection()->get_selected();
    if (selected) {
      Gtk::TreeModel::const_iterator iter = function_type_combo_.get_active();
      if (iter) {
        (*selected)[functions_columns_.function_type_] = theatre::FunctionType(
            (*iter)[function_type_columns_.function_type_]);
        (*selected)[functions_columns_.function_type_str_] =
            Glib::ustring((*iter)[function_type_columns_.function_type_str_]);
      }
    }
  });
  grid_.attach(function_type_combo_, 1, 4);

  add(grid_);

  onSelectionChanged();
}

std::vector<theatre::FixtureTypeFunction>
FixtureTypeFunctionsFrame::GetFunctions() const {
  std::vector<theatre::FixtureTypeFunction> functions;
  for (const Gtk::TreeRow& child : functions_model_->children()) {
    const size_t dmx_offset = child[functions_columns_.dmx_offset_];
    const theatre::FunctionType type = child[functions_columns_.function_type_];
    const Glib::ustring fine_channel_str =
        child[functions_columns_.fine_channel_];
    const OptionalNumber<size_t> fine_channel = GetFine(fine_channel_str);
    const size_t shape = 0;
    functions.emplace_back(type, dmx_offset, fine_channel, shape);
  }
  return functions;
}

void FixtureTypeFunctionsFrame::SetFunctions(
    const std::vector<theatre::FixtureTypeFunction>& functions) {
  functions_model_->clear();
  for (const theatre::FixtureTypeFunction& f : functions) {
    Gtk::TreeModel::iterator iter = functions_model_->append();
    const Gtk::TreeModel::Row& row = *iter;
    row[functions_columns_.dmx_offset_] = f.DmxOffset();
    row[functions_columns_.fine_channel_] = FineToString(f.FineChannelOffset());
    row[functions_columns_.function_type_] = f.Type();
    row[functions_columns_.function_type_str_] = ToString(f.Type());
  }
}

void FixtureTypeFunctionsFrame::onAdd() {
  size_t dmx_offset = 0;
  if (!functions_model_->children().empty()) {
    Gtk::TreeIter end_iter = functions_model_->children().end();
    --end_iter;
    Gtk::TreeModel::Row row = *end_iter;
    OptionalNumber<size_t> fine =
        GetFine(Glib::ustring(row[functions_columns_.fine_channel_]));
    if (fine)
      dmx_offset =
          std::max<size_t>(row[functions_columns_.dmx_offset_], *fine) + 1;
    else
      dmx_offset = row[functions_columns_.dmx_offset_] + 1;
  }

  Gtk::TreeModel::iterator iter = functions_model_->append();
  const Gtk::TreeModel::Row& row = *iter;
  row[functions_columns_.dmx_offset_] = dmx_offset;
  row[functions_columns_.fine_channel_] = "-";
  row[functions_columns_.function_type_] = theatre::FunctionType::White;
  row[functions_columns_.function_type_str_] =
      ToString(theatre::FunctionType::White);
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
  const bool is_selected = static_cast<bool>(selected);
  dmx_offset_label_.set_sensitive(is_selected);
  dmx_offset_entry_.set_sensitive(is_selected);
  fine_channel_label_.set_sensitive(is_selected);
  fine_channel_entry_.set_sensitive(is_selected);
  function_type_label_.set_sensitive(is_selected);
  function_type_combo_.set_sensitive(is_selected);
  if (is_selected) {
    dmx_offset_entry_.set_text(
        std::to_string((*selected)[functions_columns_.dmx_offset_]));
    fine_channel_entry_.set_text((*selected)[functions_columns_.fine_channel_]);
    const int ft_index = static_cast<int>(
        theatre::FunctionType((*selected)[functions_columns_.function_type_]));
    function_type_combo_.set_active(ft_index);
  } else {
    dmx_offset_entry_.set_text("");
    fine_channel_entry_.set_text("-");
    function_type_combo_.set_active(-1);
  }
}

}  // namespace glight::gui
