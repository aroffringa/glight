#include "reorderwidget.h"

namespace glight::gui::components {

ReorderWidget::ReorderWidget(theatre::Management& management,
                             EventTransmitter& hub)
    : management_(management), hub_(hub) {
  model_ = Gtk::ListStore::create(columns_);
  view_.set_model(model_);
  view_.append_column("Fixture", columns_.title_);
  view_.set_rubber_banding(true);
  view_.get_selection()->set_mode(Gtk::SelectionMode::SELECTION_MULTIPLE);
  scrolled_window_.add(view_);
  scrolled_window_.set_size_request(300, 400);
  pack_start(scrolled_window_);

  button_box_.pack_start(up_button_);
  button_box_.pack_start(down_button_);
  button_box_.pack_start(remove_button_);
  pack_start(button_box_);

  show_all_children();
}

void ReorderWidget::SetList(const std::vector<theatre::NamedObject*>& objects) {
  model_->clear();

  for (theatre::NamedObject* object : objects) {
    Gtk::TreeModel::iterator iter = model_->append();
    Gtk::TreeModel::Row row = *iter;
    row[columns_.title_] = object->Name();
    row[columns_.object_] = object;
  }
}

std::vector<theatre::NamedObject*> ReorderWidget::GetList() const {
  const auto children = model_->children();
  std::vector<theatre::NamedObject*> list;
  for (const Gtk::TreeRow& row : children) {
    list.emplace_back(row[columns_.object_]);
  }
  return list;
}

}  // namespace glight::gui::components
