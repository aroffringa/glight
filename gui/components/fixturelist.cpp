#include "fixturelist.h"

#include "gui/instance.h"

#include "theatre/fixture.h"
#include "theatre/fixturegroup.h"
#include "theatre/management.h"
#include "theatre/theatre.h"

namespace glight::gui::components {

FixtureList::FixtureList() {
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

  theatre::Management &management_ = Instance::Management();
  std::lock_guard<std::mutex> lock(management_.Mutex());
  const std::vector<system::TrackablePtr<theatre::FixtureGroup>> &groups =
      management_.FixtureGroups();
  for (const system::TrackablePtr<theatre::FixtureGroup> &group_ptr : groups) {
    theatre::FixtureGroup &group = *group_ptr;
    Gtk::TreeModel::iterator iter = model_->append();
    const Gtk::TreeModel::Row &row = *iter;
    row[columns_.title_] = group.Name();
    row[columns_.type_] = "Group";
    row[columns_.fixture_] = nullptr;
    row[columns_.group_] = &group;
  }
  const std::vector<system::TrackablePtr<theatre::Fixture>> &fixtures =
      management_.GetTheatre().Fixtures();
  for (const system::TrackablePtr<theatre::Fixture> &fixture : fixtures) {
    Gtk::TreeModel::iterator iter = model_->append();
    const Gtk::TreeModel::Row &row = *iter;
    row[columns_.title_] = fixture->Name();
    row[columns_.type_] = fixture->Type().Name();
    row[columns_.fixture_] = fixture.GetObserver();
    row[columns_.group_] = nullptr;
  }
}

void FixtureList::Select(
    const std::vector<system::ObservingPtr<theatre::Fixture>> &fixtures) {
  view_.get_selection()->unselect_all();
  Gtk::TreeModel::iterator iter;
  Gtk::TreeModel::Children children = model_->children();
  for (const auto &child : children) {
    const system::ObservingPtr<theatre::Fixture> &fixture =
        child.get_value(columns_.fixture_);
    const auto iter = std::find(fixtures.begin(), fixtures.end(), fixture);
    if (iter != fixtures.end()) view_.get_selection()->select(child);
  }
}

std::vector<system::ObservingPtr<theatre::Fixture>> FixtureList::Selection()
    const {
  Glib::RefPtr<const Gtk::TreeSelection> selection = view_.get_selection();
  std::vector<Gtk::TreeModel::Path> selected = selection->get_selected_rows();
  std::vector<system::ObservingPtr<theatre::Fixture>> result;
  for (Gtk::TreePath &row : selected) {
    theatre::FixtureGroup *group = (*model_->get_iter(row))[columns_.group_];
    if (group) {
      for (const system::ObservingPtr<theatre::Fixture> &fixture :
           group->Fixtures()) {
        result.emplace_back(fixture);
      }
    } else {
      const system::ObservingPtr<theatre::Fixture> &fixture =
          (*model_->get_iter(row))[columns_.fixture_];
      result.emplace_back(fixture);
    }
  }
  return result;
}

}  // namespace glight::gui::components
