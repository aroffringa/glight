#include "editcolorrange.h"

#include "theatre/color.h"
#include "theatre/controlvalue.h"

namespace glight::gui::windows {

using theatre::ColorRangeParameters;

EditColorRange::EditColorRange(std::vector<ColorRangeParameters::Range> ranges)
    : color_selection_(this, false), ranges_(std::move(ranges)) {
  set_default_size(400, 400);

  add_button_.signal_clicked().connect([&]() { Add(); });
  tool_bar_.append(add_button_);
  remove_button_.signal_clicked().connect([&]() { Remove(); });
  tool_bar_.append(remove_button_);
  grid_.attach(tool_bar_, 0, 0, 2, 1);

  list_model_ = Gtk::ListStore::create(list_columns_);

  list_view_.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
  list_view_.get_selection()->signal_changed().connect(
      [&]() { OnSelectionChanged(); });
  list_view_.set_model(list_model_);
  list_view_.append_column("Start", list_columns_.start_);
  list_view_.append_column("End", list_columns_.end_);
  list_view_.append_column("Color", list_columns_.color_);
  list_view_.set_rubber_banding(true);
  FillList();
  scrolled_window_.add(list_view_);

  scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  scrolled_window_.set_hexpand(true);
  scrolled_window_.set_vexpand(true);
  grid_.attach(scrolled_window_, 0, 1, 2, 1);

  start_label_.set_margin_end(5);
  start_label_.set_halign(Gtk::ALIGN_END);
  grid_.attach(start_label_, 0, 2);
  const auto save_change = [&]() { SaveChange(); };
  start_entry_.signal_changed().connect(save_change);
  grid_.attach(start_entry_, 1, 2);
  end_label_.set_margin_end(5);
  end_label_.set_halign(Gtk::ALIGN_END);
  grid_.attach(end_label_, 0, 3);
  end_entry_.signal_changed().connect(save_change);
  grid_.attach(end_entry_, 1, 3);
  color_check_button_.signal_clicked().connect(save_change);
  color_check_button_.set_margin_end(5);
  color_check_button_.set_halign(Gtk::ALIGN_END);
  grid_.attach(color_check_button_, 0, 4);
  color_selection_.SignalColorChanged().connect(save_change);
  grid_.attach(color_selection_, 1, 4);

  SetSensitive(false);

  grid_.set_hexpand(true);
  grid_.set_vexpand(true);
  add(grid_);
  show_all();
}

void EditColorRange::FillList() {
  list_model_->clear();
  for (size_t i = 0; i != ranges_.size(); ++i) {
    const ColorRangeParameters::Range& range = ranges_[i];
    Gtk::TreeModel::Row new_item = *list_model_->append();
    new_item[list_columns_.index_] = i;
    new_item[list_columns_.start_] = range.input_min;
    new_item[list_columns_.end_] = range.input_max;
    const std::string color_str = range.color ? ToString(*range.color) : "-";
    new_item[list_columns_.color_] = color_str;
  }
}

void EditColorRange::UpdateList() {
  for (auto row : list_model_->children()) {
    const ColorRangeParameters::Range& range =
        ranges_[row[list_columns_.index_]];
    row[list_columns_.start_] = range.input_min;
    row[list_columns_.end_] = range.input_max;
    const std::string color_str = range.color ? ToString(*range.color) : "-";
    row[list_columns_.color_] = color_str;
  }
}

void EditColorRange::Add() {
  if (ranges_.empty()) {
    ranges_.emplace_back(0, 256, theatre::Color::White());
  } else {
    const ColorRangeParameters::Range& last_range = ranges_.back();
    ranges_.emplace_back(last_range.input_max, 256, theatre::Color::White());
  }
  FillList();
}

void EditColorRange::Remove() {
  const std::vector<Gtk::TreeModel::Path> selection =
      list_view_.get_selection()->get_selected_rows();
  if (!selection.empty()) {
    for (auto iter = selection.rbegin(); iter != selection.rend(); ++iter) {
      const size_t range_index =
          (*list_model_->get_iter(*iter))[list_columns_.index_];
      ranges_.erase(ranges_.begin() + range_index);
    }
  }
  FillList();
}

void EditColorRange::OnSelectionChanged() {
  const std::vector<Gtk::TreeModel::Path> selection =
      list_view_.get_selection()->get_selected_rows();
  RecursionLock::Token token(lock_);
  if (selection.empty()) {
    SetSensitive(false);
    start_entry_.set_text("");
    end_entry_.set_text("");
    color_check_button_.set_active(false);
    color_selection_.SetColor(theatre::Color::White());
  } else {
    SetSensitive(true);
    const size_t range_index =
        (*list_model_->get_iter(selection.front()))[list_columns_.index_];
    const ColorRangeParameters::Range& range = ranges_[range_index];
    start_entry_.set_text(std::to_string(range.input_min));
    end_entry_.set_text(std::to_string(range.input_max));
    color_check_button_.set_active(range.color.has_value());
    if (range.color) {
      color_selection_.SetColor(*range.color);
    } else {
      color_selection_.SetColor(theatre::Color::White());
    }
  }
}

void EditColorRange::SaveChange() {
  if (lock_.IsFirst()) {
    const std::vector<Gtk::TreeModel::Path> selection =
        list_view_.get_selection()->get_selected_rows();
    for (const Gtk::TreeModel::Path& path : selection) {
      const size_t range_index =
          (*list_model_->get_iter(path))[list_columns_.index_];
      ColorRangeParameters::Range& range = ranges_[range_index];

      range.input_min = std::max(0, std::atoi(start_entry_.get_text().c_str()));
      range.input_max = std::max<unsigned>(
          range.input_min, std::atoi(end_entry_.get_text().c_str()));
      if (color_check_button_.get_active())
        range.color = color_selection_.GetColor();
      else
        range.color = {};
    }
    UpdateList();
  }
}

std::vector<ColorRangeParameters::Range> EditColorRange::GetRanges() const {
  std::vector<ColorRangeParameters::Range> ranges = ranges_;
  std::sort(ranges.begin(), ranges.end(),
            [](const ColorRangeParameters::Range& a,
               const ColorRangeParameters::Range& b) {
              return a.input_min < b.input_min;
            });
  for (size_t i = 1; i < ranges.size(); ++i) {
    ranges[i].input_min =
        std::max(ranges[i].input_min, ranges[i - 1].input_max);
    ranges[i].input_max = std::max(ranges[i].input_min, ranges[i].input_max);
  }
  return ranges;
}

}  // namespace glight::gui::windows
