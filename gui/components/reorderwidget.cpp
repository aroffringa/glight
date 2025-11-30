#include "reorderwidget.h"

#include "../../theatre/fixture.h"
#include "../../theatre/fixturemode.h"
#include "../../theatre/fixturetype.h"

#include <ranges>

namespace glight::gui::components {

ReorderWidget::ReorderWidget() {
  model_ = Gtk::ListStore::create(columns_);
  view_.set_model(model_);
  SetColumns();
  view_.set_rubber_banding(true);
  view_.get_selection()->set_mode(Gtk::SelectionMode::MULTIPLE);
  scrolled_window_.set_child(view_);
  scrolled_window_.set_size_request(300, 400);
  append(scrolled_window_);

  button_box_.set_homogeneous(true);
  button_box_.set_valign(Gtk::Align::CENTER);
  up_button_.set_image_from_icon_name("go-up");
  up_button_.signal_clicked().connect([&]() { MoveUp(); });
  button_box_.append(up_button_);
  down_button_.set_image_from_icon_name("go-down");
  down_button_.signal_clicked().connect([&]() { MoveDown(); });
  button_box_.append(down_button_);
  remove_button_.set_image_from_icon_name("edit-delete");
  remove_button_.signal_clicked().connect([&]() { Remove(); });
  button_box_.append(remove_button_);
  append(button_box_);
}

void ReorderWidget::Append(system::ObservingPtr<theatre::NamedObject> object) {
  Gtk::TreeModel::iterator iter = model_->append();
  Gtk::TreeModel::Row& row = *iter;
  row[columns_.title_] = object->Name();
  if (const theatre::Fixture* fixture =
          dynamic_cast<const theatre::Fixture*>(object.Get());
      fixture) {
    row[columns_.type_] = fixture->Mode().Type().Name();
  }
  row[columns_.object_] = std::move(object);
  signal_changed_();
}

std::vector<system::ObservingPtr<theatre::NamedObject>> ReorderWidget::GetList()
    const {
  auto children = model_->children();
  std::vector<system::ObservingPtr<theatre::NamedObject>> list;
  for (Gtk::TreeRow& row : children) {
    list.emplace_back(row[columns_.object_]);
  }
  return list;
}

void ReorderWidget::MoveUp() {
  std::vector<Gtk::TreeModel::Path> rows =
      view_.get_selection()->get_selected_rows();
  for (const Gtk::TreeModel::Path& row : rows) {
    Gtk::TreeModel::iterator iter = model_->get_iter(row);
    Gtk::TreeModel::iterator previous_iter = iter;
    --previous_iter;
    if (previous_iter) model_->iter_swap(iter, previous_iter);
  }
  signal_changed_();
}

void ReorderWidget::MoveDown() {
  std::vector<Gtk::TreeModel::Path> rows =
      view_.get_selection()->get_selected_rows();
  std::ranges::reverse_view rev_view(rows);
  for (const Gtk::TreeModel::Path& row : rev_view) {
    Gtk::TreeModel::iterator iter = model_->get_iter(row);
    Gtk::TreeModel::iterator next_iter = iter;
    ++next_iter;
    if (next_iter) model_->iter_swap(iter, next_iter);
  }
  signal_changed_();
}

void ReorderWidget::Remove() {
  std::vector<Gtk::TreeModel::Path> rows =
      view_.get_selection()->get_selected_rows();
  for (const Gtk::TreeModel::Path& row : rows) {
    model_->erase(model_->get_iter(row));
  }
  signal_changed_();
}

void ReorderWidget::SetColumns() {
  view_.remove_all_columns();
  view_.append_column("Fixture", columns_.title_);
  if (show_type_column_) view_.append_column("Type", columns_.type_);
}

}  // namespace glight::gui::components
