#ifndef STRING_INPUT_DIALOG_H_
#define STRING_INPUT_DIALOG_H_

#include <cstdlib>
#include <sstream>
#include <string>

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/stock.h>

namespace glight::gui {

class StringInputDialog : public Gtk::Dialog {
 public:
  StringInputDialog(const Glib::ustring& title,
                    const Glib::ustring& value_caption,
                    const std::string& default_value)
      : Dialog(title, true), label_(value_caption) {
    h_box_.pack_start(label_);

    std::ostringstream s;
    s << default_value;
    entry_.set_text(s.str());
    entry_.set_activates_default(true);
    h_box_.pack_end(entry_);

    get_content_area()->pack_start(h_box_);
    h_box_.show_all();

    add_button("_Ok", Gtk::RESPONSE_OK);
    add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    set_default_response(Gtk::RESPONSE_OK);
  }

  std::string Value() const { return entry_.get_text(); }

 private:
  Gtk::Label label_;
  Gtk::Entry entry_;
  Gtk::HBox h_box_;
};

}  // namespace glight::gui

#endif  // NUMINPUTDIALOG_H
