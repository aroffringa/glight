#include "fixturetypeswindow.h"

#include "../eventtransmitter.h"
#include "../fixtureselection.h"

#include <gtkmm/main.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include "../../theatre/fixture.h"
#include "../../theatre/fixturecontrol.h"
#include "../../theatre/folder.h"
#include "../../theatre/management.h"
#include "../../theatre/theatre.h"

namespace glight::gui {

FixtureTypesWindow::FixtureTypesWindow(EventTransmitter *eventHub,
                                       theatre::Management &management)
    : event_hub_(eventHub),
      management_(&management),
      name_label_("Name:"),
      class_label_("Class:"),
      new_button_("New"),
      remove_button_("Remove"),
      save_button_("Save") {
  set_title("Glight - fixture types");
  set_size_request(200, 400);

  change_management_connection_ = event_hub_->SignalChangeManagement().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onChangeManagement));
  update_controllables_connection_ =
      event_hub_->SignalUpdateControllables().connect(
          [&]() { FixtureTypesWindow::update(); });

  // Left part
  list_model_ = Gtk::ListStore::create(list_columns_);

  list_view_.set_model(list_model_);
  list_view_.append_column("Name", list_columns_.name_);
  list_view_.append_column("Used", list_columns_.in_use_);
  list_view_.append_column("Functions", list_columns_.functions_);
  list_view_.get_selection()->signal_changed().connect(
      [&]() { onSelectionChanged(); });
  fillList();
  scrolled_window_.add(list_view_);

  scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  left_box_.pack_start(scrolled_window_);

  paned_.add1(left_box_);
  left_box_.set_hexpand(true);
  left_box_.set_vexpand(true);

  // Right part
  right_grid_.attach(name_label_, 0, 0);
  right_grid_.attach(name_entry_, 1, 0);
  name_entry_.set_hexpand(true);

  right_grid_.attach(class_label_, 0, 1);
  const std::vector<theatre::FixtureClass> classes =
      theatre::FixtureType::GetClassList();
  for (theatre::FixtureClass c : classes)
    class_combo_.append(theatre::FixtureType::ClassName(c));
  right_grid_.attach(class_combo_, 1, 1);
  class_combo_.set_hexpand(true);
  right_grid_.attach(functions_frame_, 0, 2, 2, 1);
  functions_frame_.set_vexpand(true);
  functions_frame_.set_hexpand(true);

  paned_.add(right_grid_);
  right_grid_.set_hexpand(true);
  right_grid_.set_vexpand(true);
  right_grid_.set_row_spacing(5);
  right_grid_.set_column_spacing(5);

  main_grid_.attach(paned_, 0, 0, 1, 1);

  // Buttons at bottom
  new_button_.set_image_from_icon_name("document-new");
  new_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onNewButtonClicked), false);
  button_box_.pack_start(new_button_);

  remove_button_.set_image_from_icon_name("edit-delete");
  remove_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onRemoveClicked));
  button_box_.pack_start(remove_button_);

  save_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onSaveClicked));
  button_box_.pack_start(save_button_);

  main_grid_.attach(button_box_, 0, 1, 1, 1);
  button_box_.set_hexpand(true);
  button_box_.set_vexpand(false);

  add(main_grid_);
  main_grid_.show_all();

  onSelectionChanged();
}

FixtureTypesWindow::~FixtureTypesWindow() {
  change_management_connection_.disconnect();
  update_controllables_connection_.disconnect();
}

void FixtureTypesWindow::fillList() {
  list_model_->clear();

  std::lock_guard<std::mutex> lock(management_->Mutex());
  const std::vector<std::unique_ptr<theatre::FixtureType>> &types =
      management_->GetTheatre().FixtureTypes();
  for (const std::unique_ptr<theatre::FixtureType> &type : types) {
    Gtk::TreeModel::iterator iter = list_model_->append();
    Gtk::TreeModel::Row row = *iter;
    row[list_columns_.fixture_type_] = type.get();
    row[list_columns_.name_] = type->Name();
    row[list_columns_.functions_] = FunctionSummary(*type);
    row[list_columns_.in_use_] = management_->GetTheatre().IsUsed(*type);
  }
}

void FixtureTypesWindow::onNewButtonClicked() {
  if (!list_model_->children().empty()) {
    Gtk::TreeModel::Row row = *list_model_->children().rbegin();
    // If the last row is already a new, unsaved type, ignore the request
    if (row[list_columns_.fixture_type_] == nullptr) return;
  }
  Gtk::TreeModel::iterator iter = list_model_->append();
  Gtk::TreeModel::Row row = *iter;
  row[list_columns_.fixture_type_] = nullptr;
  row[list_columns_.name_] = {};
  row[list_columns_.functions_] = {};
  row[list_columns_.in_use_] = false;
  list_view_.get_selection()->select(iter);
}

void FixtureTypesWindow::onRemoveClicked() {
  theatre::FixtureType *type = getSelected();
  if (type) {
    management_->RemoveFixtureType(*type);
    event_hub_->EmitUpdate();
  } else {
    const Gtk::TreeModel::const_iterator selected =
        list_view_.get_selection()->get_selected();
    if (selected) list_model_->erase(selected);
  }
}

void FixtureTypesWindow::onSaveClicked() {
  theatre::FixtureType *type = getSelected();
  if (type) {
    if (management_->GetTheatre().IsUsed(*type)) {
      Gtk::MessageDialog dialog(*this,
                                "A used type can not be changed: first remove "
                                "all fixtures of this type");
      dialog.run();
    }
  } else {
    theatre::FixtureType ft;
    type = &management_->GetTheatre().AddFixtureType(ft);
    management_->RootFolder().Add(*type);
  }
  type->SetName(name_entry_.get_text());
  type->SetFixtureClass(
      theatre::FixtureType::NameToClass(class_combo_.get_active_text()));
  type->SetFunctions(functions_frame_.GetFunctions());
  event_hub_->EmitUpdate();
}

theatre::FixtureType *FixtureTypesWindow::getSelected() {
  Glib::RefPtr<Gtk::TreeSelection> selection = list_view_.get_selection();
  const Gtk::TreeModel::const_iterator selected = selection->get_selected();
  if (selected)
    return (*selected)[list_columns_.fixture_type_];
  else
    return nullptr;
}

void FixtureTypesWindow::onSelectionChanged() {
  if (recursion_lock_.IsFirst()) {
    RecursionLock::Token token(recursion_lock_);
    Glib::RefPtr<Gtk::TreeSelection> selection = list_view_.get_selection();
    const Gtk::TreeModel::const_iterator selected = selection->get_selected();
    const bool has_selection = selected;
    const theatre::FixtureType *type =
        has_selection ? static_cast<theatre::FixtureType *>(
                            (*selected)[list_columns_.fixture_type_])
                      : nullptr;
    remove_button_.set_sensitive(has_selection);
    save_button_.set_sensitive(has_selection);
    right_grid_.set_sensitive(has_selection);
    if (type) {
      class_combo_.set_active_text(
          theatre::FixtureType::ClassName(type->GetFixtureClass()));
      functions_frame_.SetFunctions(type->Functions());
    } else {
      class_combo_.set_active_text(
          theatre::FixtureType::ClassName(theatre::FixtureClass::Par));
      functions_frame_.SetFunctions({});
    }
  }
}

}  // namespace glight::gui
