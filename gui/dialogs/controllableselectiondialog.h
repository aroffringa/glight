#ifndef GUI_CONTROLLABLE_SELECT_DIALOG_H_
#define GUI_CONTROLLABLE_SELECT_DIALOG_H_

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/dialog.h>

#include "../../theatre/forwards.h"

#include "../components/objectbrowser.h"

namespace glight::gui {

class EventTransmitter;

class ControllableSelectionDialog : public Gtk::Dialog {
 public:
  ControllableSelectionDialog(const std::string& title, bool show_new_button)
      : Dialog(title, true), object_browser_(), new_button_("New...") {
    set_size_request(600, 400);

    object_browser_.SignalSelectionChange().connect(
        [&]() { OnSelectionChanged(); });
    get_content_area()->pack_start(object_browser_);
    object_browser_.show_all();

    new_button_.signal_clicked().connect([&]() { OnNew(); });
    get_content_area()->pack_start(new_button_, false, false);

    add_button("Cancel", Gtk::RESPONSE_CANCEL);
    select_button_ = add_button("Select", Gtk::RESPONSE_OK);
    select_button_->set_sensitive(false);
  }

  void SelectObject(theatre::FolderObject& object) {
    object_browser_.SelectObject(object);
  }

  theatre::FolderObject* SelectedObject() {
    return object_browser_.SelectedObject();
  }
  theatre::Folder& SelectedFolder() { return object_browser_.SelectedFolder(); }

  void ShowNewButton(bool make_visible) {
    new_button_.set_visible(make_visible);
  }

  sigc::signal<void()>& SignalNewClicked() { return signal_new_clicked_; }

  void SetFilter(ObjectListType filter) {
    object_browser_.SetDisplayType(filter);
  }

 private:
  void OnSelectionChanged() {
    select_button_->set_sensitive(object_browser_.SelectedObject());
  }
  void OnNew() { signal_new_clicked_(); }
  ObjectBrowser object_browser_;
  Gtk::Button new_button_;
  Gtk::Button* select_button_;
  sigc::signal<void()> signal_new_clicked_;
};

}  // namespace glight::gui

#endif
