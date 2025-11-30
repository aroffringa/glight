#include "../components/colorsequencewidget.h"

#include <gtkmm/window.h>

namespace glight::gui {

class GradientWindow : public Gtk::Window {
 public:
  GradientWindow(size_t maxCount)
      : _colorSequence(this, false, false),
        _cancelButton("Cancel"),
        _okayButton("Okay"),
        _result(false) {
    set_size_request(250, 300);
    _colorSequence.SetMinCount(2);
    _colorSequence.SetMaxCount(maxCount);
    _colorSequence.SetAllowVariables(false);
    _box.append(_colorSequence);

    _buttonBox.set_homogeneous(true);

    _cancelButton.signal_clicked().connect([&] { hide(); });
    _buttonBox.append(_cancelButton);
    _okayButton.signal_clicked().connect([&] {
      _result = true;
      hide();
    });
    _buttonBox.append(_okayButton);

    _box.append(_buttonBox);

    set_child(_box);
  }

  bool Result() const { return _result; }

  std::vector<Color> GetColors() const { return _colorSequence.GetColors(); }

 private:
  Gtk::Box _box{Gtk::Orientation::VERTICAL};
  ColorSequenceWidget _colorSequence;
  Gtk::Box _buttonBox;
  Gtk::Button _cancelButton, _okayButton;
  bool _result;
};

}  // namespace glight::gui
