#include "../components/colorsequencewidget.h"

#include <gtkmm/window.h>

class GradientWindow : public Gtk::Window {
 public:
  GradientWindow(size_t maxCount)
      : _colorSequence(this, false),
        _cancelButton("Cancel"),
        _okayButton("Okay"),
        _result(false) {
    set_size_request(250, 300);
    _colorSequence.SetMinCount(2);
    _colorSequence.SetMaxCount(maxCount);
    _box.pack_start(_colorSequence);

    _cancelButton.signal_clicked().connect([&] { hide(); });
    _buttonBox.pack_start(_cancelButton);
    _okayButton.signal_clicked().connect([&] {
      _result = true;
      hide();
    });
    _buttonBox.pack_start(_okayButton);

    _box.pack_start(_buttonBox);

    add(_box);
    show_all_children();
  }

  bool Result() const { return _result; }

  std::vector<Color> GetColors() const { return _colorSequence.GetColors(); }

 private:
  Gtk::VBox _box;
  ColorSequenceWidget _colorSequence;
  Gtk::ButtonBox _buttonBox;
  Gtk::Button _cancelButton, _okayButton;
  bool _result;
};
