#include "colorsequencewidget.h"

#include <algorithm>
#include <random>

#include <glibmm/main.h>
#include <gtkmm/main.h>

#include "../windows/gradientwindow.h"

#include "../../theatre/design/colorsequences.h"

namespace glight::gui {

ColorSequenceWidget::ColorSequenceWidget(Gtk::Window *parent,
                                         bool showGradientButton,
                                         bool showShuffleButton)
    : _frame("Colors"),
      _allEqual("Use one color for all"),
      _plusButton("+"),
      _gradientButton("Gradient"),
      _shuffleButton("Shuffle"),
      _minButton("-"),
      _loadDefaultButton("Load"),
      _parent(parent),
      _minCount(1),
      _maxCount(0) {
  _allEqual.signal_clicked().connect(
      sigc::mem_fun(*this, &ColorSequenceWidget::onToggleEqual));
  pack_start(_allEqual, false, false);

  _minButton.set_sensitive(false);
  _minButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ColorSequenceWidget::onDecreaseColors));
  _buttonBox.pack_start(_minButton);

  if (showGradientButton) {
    _gradientButton.set_sensitive(false);
    _gradientButton.signal_clicked().connect(
        sigc::mem_fun(*this, &ColorSequenceWidget::onGradient));
    _buttonBox.pack_start(_gradientButton);
  }

  if (showShuffleButton) {
    _shuffleButton.set_sensitive(false);
    _shuffleButton.signal_clicked().connect([&]() { Shuffle(); });
    _buttonBox.pack_start(_shuffleButton);
  }

  _plusButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ColorSequenceWidget::onIncreaseColors));
  _buttonBox.pack_start(_plusButton);

  pack_start(_buttonBox, false, false);

  _loadDefaultList = Gtk::ListStore::create(_listColumns);
  const std::vector<std::string> list = theatre::GetDefaultColorSequences();
  for (std::string name : list) {
    Gtk::TreeIter iter = _loadDefaultList->append();
    (*iter)[_listColumns._title] = name;
  }
  _loadDefaultCombo.set_model(_loadDefaultList);
  _loadDefaultCombo.set_entry_text_column(_listColumns._title);
  _loadDefaultCombo.pack_start(_listColumns._title);
  _loadDefaultCombo.set_active(0);
  _loadDefaultBox.pack_start(_loadDefaultCombo);

  _loadDefaultButton.signal_clicked().connect([&]() { LoadDefault(); });
  _loadDefaultBox.pack_end(_loadDefaultButton);
  pack_start(_loadDefaultBox, false, false);

  _widgets.emplace_back(std::make_unique<ColorSelectWidget>(_parent));
  _box.pack_start(*_widgets.back(), true, false);
  _widgets.back()->SignalColorChanged().connect(
      sigc::mem_fun(*this, &ColorSequenceWidget::onFirstColorChange));

  _scrolledWindow.add(_box);
  _frame.add(_scrolledWindow);
  pack_start(_frame, true, true);

  show_all_children();
}

void ColorSequenceWidget::onGradient() {
  if (!_allEqual.get_active() && _widgets.size() > 2) {
    GradientWindow window(_widgets.size());
    window.set_modal(true);
    Gtk::Main::run(window);

    if (window.Result()) {
      std::vector<Color> colors = window.GetColors();
      _widgets.front()->SetColor(colors.front());
      _widgets.back()->SetColor(colors.back());

      for (size_t i = 1; i < _widgets.size() - 1; ++i) {
        double floatIndex = static_cast<double>(i) * (colors.size() - 1) /
                            (_widgets.size() - 1);
        const Color leftColor = colors[floor(floatIndex)];
        const Color rightColor = colors[floor(floatIndex) + 1];
        const double balance = floatIndex - floor(floatIndex);
        const unsigned red =
            (rightColor.Red() * balance + leftColor.Red() * (1.0 - balance));
        const unsigned green = (rightColor.Green() * balance +
                                leftColor.Green() * (1.0 - balance));
        const unsigned blue =
            (rightColor.Blue() * balance + leftColor.Blue() * (1.0 - balance));
        _widgets[i]->SetColor(Color(red, green, blue));
      }
    }
  }
}

void ColorSequenceWidget::Shuffle() {
  std::vector<glight::theatre::Color> colors = GetColors();
  std::shuffle(colors.begin(), colors.end(), std::random_device());
  SetColors(colors);
}

void ColorSequenceWidget::LoadDefault() {
  Gtk::TreeIter active = _loadDefaultCombo.get_active();
  const std::string default_name =
      Glib::ustring((*active)[_listColumns._title]);
  const std::vector<theatre::Color> colors =
      theatre::GetDefaultColorSequence(default_name, _widgets.size());
  if (!colors.empty()) SetColors(colors);
}

}  // namespace glight::gui
