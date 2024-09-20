#include "fixturetypefunctionsframe.h"

#include <cassert>

#include "editcolorrange.h"

namespace glight::gui {

using system::OptionalNumber;
using theatre::FixtureTypeFunction;
using theatre::FunctionType;

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

FixtureTypeFunctionsFrame::FixtureTypeFunctionsFrame(Gtk::Window& parent_window)
    : Gtk::Frame("Functions"), parent_window_(parent_window) {
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
  grid_.attach(functions_scrollbars_, 0, 0, 3, 1);

  add_function_button_.signal_clicked().connect([&]() { onAdd(); });
  functions_button_box_.pack_start(add_function_button_);
  remove_function_button_.signal_clicked().connect([&]() { onRemove(); });
  functions_button_box_.pack_end(remove_function_button_);
  grid_.attach(functions_button_box_, 0, 1, 3, 1);
  grid_.set_hexpand(true);
  grid_.set_vexpand(true);

  grid_.attach(dmx_offset_label_, 0, 2);
  dmx_offset_entry_.signal_changed().connect([&]() {
    Gtk::TreeModel::iterator selected =
        functions_view_.get_selection()->get_selected();
    if (selected) {
      const int val =
          std::clamp(std::atoi(dmx_offset_entry_.get_text().c_str()), 0, 511);
      (*selected)[functions_columns_.dmx_offset_] = val;
      (*(*selected)[functions_columns_.function_]).SetDmxOffset(val);
    }
  });
  grid_.attach(dmx_offset_entry_, 1, 2, 2, 1);
  fine_channel_entry_.signal_changed().connect([&]() {
    Gtk::TreeModel::iterator selected =
        functions_view_.get_selection()->get_selected();
    if (selected) {
      const OptionalNumber<size_t> fine =
          GetFine(fine_channel_entry_.get_text());
      (*selected)[functions_columns_.fine_channel_] = FineToString(fine);
      (*(*selected)[functions_columns_.function_]).SetFineChannelOffset(fine);
    }
  });
  grid_.attach(fine_channel_label_, 0, 3);
  grid_.attach(fine_channel_entry_, 1, 3, 2, 1);
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
        FixtureTypeFunction* function =
            (*selected)[functions_columns_.function_];
        FunctionType type = (*iter)[function_type_columns_.function_type_];
        // We should not update the type if no change is made, as it would
        // also destroy the parameters of the type (like macro color range)
        if (type != function->Type()) {
          function->SetType(type);
          (*selected)[functions_columns_.function_type_str_] =
              Glib::ustring((*iter)[function_type_columns_.function_type_str_]);
        }
      }
    }
  });
  grid_.attach(function_type_combo_, 1, 4);

  function_parameters_button_.signal_clicked().connect(
      [&]() { OpenFunctionParametersEditWindow(); });
  function_parameters_button_.set_hexpand(false);
  grid_.attach(function_parameters_button_, 2, 4);

  grid_.attach(power_label_, 0, 5);
  grid_.attach(power_entry_, 1, 5, 2, 1);
  power_entry_.signal_changed().connect([&]() {
    Gtk::TreeModel::iterator selected =
        functions_view_.get_selection()->get_selected();
    if (selected) {
      const unsigned val =
          std::max(0LL, std::atoll(power_entry_.get_text().c_str()));
      (*(*selected)[functions_columns_.function_]).SetPower(val);
    }
  });

  add(grid_);

  onSelectionChanged();
}

void FixtureTypeFunctionsFrame::FillModel() {
  functions_model_->clear();
  for (FixtureTypeFunction& f : functions_) {
    Gtk::TreeModel::iterator iter = functions_model_->append();
    const Gtk::TreeModel::Row& row = *iter;
    row[functions_columns_.dmx_offset_] = f.DmxOffset();
    row[functions_columns_.fine_channel_] = FineToString(f.FineChannelOffset());
    row[functions_columns_.function_] = &f;
    row[functions_columns_.function_type_str_] = ToString(f.Type());
  }
}

void FixtureTypeFunctionsFrame::UpdateModel() {
  auto row_iter = functions_model_->children().begin();
  for (FixtureTypeFunction& f : functions_) {
    const Gtk::TreeModel::Row& row = *row_iter;
    row[functions_columns_.dmx_offset_] = f.DmxOffset();
    row[functions_columns_.fine_channel_] = FineToString(f.FineChannelOffset());
    row[functions_columns_.function_] = &f;
    row[functions_columns_.function_type_str_] = ToString(f.Type());
    ++row_iter;
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

  // This might change the address of the functions, so we need
  // to call UpdateModel() after adding the element.
  functions_.emplace_back(theatre::FunctionType::White, dmx_offset,
                          system::OptionalNumber<size_t>(), 0);
  functions_model_->append();
  UpdateModel();
}

void FixtureTypeFunctionsFrame::onRemove() {
  Glib::RefPtr<Gtk::TreeSelection> selection = functions_view_.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected) {
    FixtureTypeFunction* function = (*selected)[functions_columns_.function_];
    auto iter =
        std::find_if(functions_.begin(), functions_.end(),
                     [function](const FixtureTypeFunction& ftf) -> bool {
                       return &ftf == function;
                     });
    assert(iter != functions_.end());
    // This might change the address of functions, so we need
    // to call UpdateModel() after removing the element.
    functions_.erase(iter);
    functions_model_->erase(selected);
    UpdateModel();
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
  function_parameters_button_.set_sensitive(is_selected);
  if (is_selected) {
    const FixtureTypeFunction& function =
        *(*selected)[functions_columns_.function_];
    dmx_offset_entry_.set_text(std::to_string(function.DmxOffset()));
    fine_channel_entry_.set_text((*selected)[functions_columns_.fine_channel_]);
    const int ft_index = static_cast<int>(function.Type());
    function_type_combo_.set_active(ft_index);
    power_entry_.set_text(std::to_string(function.Power()));
  } else {
    dmx_offset_entry_.set_text("");
    fine_channel_entry_.set_text("-");
    function_type_combo_.set_active(-1);
    power_entry_.set_text("-");
  }
}

void FixtureTypeFunctionsFrame::OpenFunctionParametersEditWindow() {
  Gtk::TreeModel::iterator selected =
      functions_view_.get_selection()->get_selected();
  const bool is_selected = static_cast<bool>(selected);
  if (is_selected) {
    FixtureTypeFunction& function = *(*selected)[functions_columns_.function_];
    if (function.Type() == FunctionType::ColorMacro ||
        function.Type() == FunctionType::ColorWheel) {
      sub_window_ = std::make_unique<windows::EditColorRange>(
          function.GetColorRangeParameters().GetRanges());
      sub_window_->set_modal(true);
      sub_window_->set_transient_for(parent_window_);
      sub_window_->signal_hide().connect([&]() {
        function.GetColorRangeParameters().GetRanges() =
            static_cast<windows::EditColorRange*>(sub_window_.get())
                ->GetRanges();
        sub_window_.reset();
      });
      sub_window_->show();
    }
  }
}

}  // namespace glight::gui
