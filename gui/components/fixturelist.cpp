#include "fixturelist.h"

#include "../../theatre/fixture.h"
#include "../../theatre/management.h"
#include "../../theatre/theatre.h"

namespace glight::gui::components {

FixtureList::FixtureList(theatre::Management &management, EventTransmitter &hub)
    : management_(management), hub_(hub) {
  model_ = Gtk::ListStore::create(columns_);
  view_.set_model(model_);
  view_.append_column("Fixture", columns_.title_);
  view_.append_column("Type", columns_.type_);
  view_.set_rubber_banding(true);
  view_.get_selection()->set_mode(Gtk::SelectionMode::SELECTION_MULTIPLE);
  Fill();
  add(view_);
  set_size_request(300, 400);
}

void FixtureList::Fill() {
  model_->clear();

  std::lock_guard<std::mutex> lock(management_.Mutex());
  const std::vector<std::unique_ptr<theatre::Fixture>> &fixtures =
      management_.GetTheatre().Fixtures();
  for (const std::unique_ptr<theatre::Fixture> &fixture : fixtures) {
    Gtk::TreeModel::iterator iter = model_->append();
    const Gtk::TreeModel::Row& row = *iter;
    row[columns_.title_] = fixture->Name();
    row[columns_.type_] = fixture->Type().Name();
    row[columns_.fixture_] = fixture.get();
  }
}

void FixtureList::Select(const std::vector<theatre::Fixture *> &fixtures) {
  view_.get_selection()->unselect_all();
  Gtk::TreeModel::iterator iter;
  Gtk::TreeModel::Children children = model_->children();
  for (const auto &child : children) {
    const theatre::Fixture *fixture = child.get_value(columns_.fixture_);
    const auto iter = std::find(fixtures.begin(), fixtures.end(), fixture);
    if (iter != fixtures.end()) view_.get_selection()->select(child);
  }
}

std::vector<theatre::Fixture *> FixtureList::Selection() const {
  Glib::RefPtr<const Gtk::TreeSelection> selection = view_.get_selection();
  std::vector<Gtk::TreeModel::Path> selected = selection->get_selected_rows();
  std::vector<theatre::Fixture *> result;
  for (Gtk::TreePath &row : selected) {
    theatre::Fixture *fixture = (*model_->get_iter(row))[columns_.fixture_];
    result.emplace_back(fixture);
  }
  return result;
}

}  // namespace glight::gui::components
