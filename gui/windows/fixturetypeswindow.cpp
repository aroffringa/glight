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

FixtureTypesWindow::FixtureTypesWindow(EventTransmitter *eventHub,
                                       Management &management)
    : event_hub_(eventHub),
      management_(&management),
      new_button_("New"),
      remove_button_("Remove") {
  set_title("Glight - fixture types");
  set_size_request(200, 400);

  change_management_connection_ = event_hub_->SignalChangeManagement().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onChangeManagement));
  update_controllables_connection_ =
      event_hub_->SignalUpdateControllables().connect(
          [&]() { FixtureTypesWindow::update(); });

  list_model_ = Gtk::ListStore::create(list_columns_);

  list_view_.get_selection()->signal_changed().connect(
      [&]() { onSelectionChanged(); });
  list_view_.set_model(list_model_);
  list_view_.append_column("Name", list_columns_.name_);
  list_view_.append_column("Functions", list_columns_.functions_);
  fillList();
  scrolled_window_.add(list_view_);

  scrolled_window_.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
  main_box_.pack_start(scrolled_window_);

  new_button_.set_image_from_icon_name("document-new");
  new_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onNewButtonClicked), false);
  button_box_.pack_start(new_button_);

  remove_button_.set_image_from_icon_name("edit-delete");
  remove_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &FixtureTypesWindow::onRemoveButtonClicked));
  button_box_.pack_start(remove_button_);

  main_box_.pack_start(button_box_, false, false, 0);

  add(main_box_);
  main_box_.show_all();
}

FixtureTypesWindow::~FixtureTypesWindow() {
  change_management_connection_.disconnect();
  update_controllables_connection_.disconnect();
}

void FixtureTypesWindow::fillList() {
  list_model_->clear();

  std::lock_guard<std::mutex> lock(management_->Mutex());
  const std::vector<std::unique_ptr<FixtureType>> &types =
      management_->GetTheatre().FixtureTypes();
  for (const std::unique_ptr<FixtureType> &type : types) {
    Gtk::TreeModel::iterator iter = list_model_->append();
    Gtk::TreeModel::Row row = *iter;
    row[list_columns_.fixture_type_] = type.get();
    row[list_columns_.name_] = type->Name();
    row[list_columns_.functions_] = type->Name();
  }
}

void FixtureTypesWindow::onNewButtonClicked() {}

void FixtureTypesWindow::onRemoveButtonClicked() {
  FixtureType *type = getSelected();
  if (type) {
    management_->RemoveFixtureType(*type);
    event_hub_->EmitUpdate();
  }
}

FixtureType *FixtureTypesWindow::getSelected() {
  Glib::RefPtr<Gtk::TreeSelection> selection = list_view_.get_selection();
  Gtk::TreeModel::iterator selected = selection->get_selected();
  if (selected)
    return (*selected)[list_columns_.fixture_type_];
  else
    return nullptr;
}

void FixtureTypesWindow::onSelectionChanged() {
  if (recursion_lock_.IsFirst()) {
    RecursionLock::Token token(recursion_lock_);
    FixtureType *type = getSelected();
  }
}
