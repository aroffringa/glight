#ifndef STRING_INPUT_DIALOG_H_
#define STRING_INPUT_DIALOG_H_

#include <cstdlib>
#include <sstream>
#include <string>

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

namespace glight::gui {

class StringInputDialog : public Gtk::Dialog {
 public:
  StringInputDialog(const Glib::ustring& title,
                    const Glib::ustring& value_caption,
                    const std::string& default_value)
      : Dialog(title, true), label_(value_caption) {
    h_box_.append(label_);

    entry_.set_text(default_value);
    entry_.set_activates_default(true);
    h_box_.append(entry_);

    get_content_area()->append(h_box_);

    add_button("_Ok", Gtk::ResponseType::OK);
    add_button("_Cancel", Gtk::ResponseType::CANCEL);
    set_default_response(Gtk::ResponseType::OK);
  }

  std::string Value() const { return entry_.get_text(); }

 private:
  Gtk::Label label_;
  Gtk::Entry entry_;
  Gtk::Box h_box_;
};

}  // namespace glight::gui

#endif  // NUMINPUTDIALOG_H
