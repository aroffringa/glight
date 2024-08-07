#include "colorsequencewidget.h"

#include <algorithm>
#include <random>

#include <glibmm/main.h>
#include <gtkmm/main.h>

#include "gui/windows/gradientwindow.h"
#include "gui/eventtransmitter.h"

#include "theatre/management.h"
#include "theatre/design/colorsequences.h"

namespace glight::gui {

ColorSequenceWidget::ColorSequenceWidget(Gtk::Window *parent,
                                         bool showGradientButton,
                                         bool showShuffleButton)
    : _frame("Colors"),
      _allEqual("Use one color for all"),
      _oddEvenButton("Odd/even"),
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
  _sequencingBox.pack_start(_allEqual, false, false);
  _oddEvenButton.signal_clicked().connect([&]() { OnOddEven(); });
  _sequencingBox.pack_end(_oddEvenButton, false, false);

  pack_start(_sequencingBox, false, false);
  _sequencingBox.show_all();

  _buttonBox.set_homogeneous(true);

  _minButton.set_sensitive(false);
  _minButton.signal_clicked().connect(
      sigc::mem_fun(*this, &ColorSequenceWidget::onDecreaseColors));
  _buttonBox.pack_start(_minButton);

  if (showGradientButton) {
    _gradientButton.set_sensitive(false);
    _gradientButton.signal_clicked().connect(
        sigc::mem_fun(*this, &ColorSequenceWidget::OnGradient));
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
  _buttonBox.show_all();

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
  _loadDefaultBox.show_all();

  _widgets.emplace_back(std::make_unique<ColorSelectWidget>(_parent, true));
  _box.pack_start(*_widgets.back(), true, false);
  _widgets.back()->SignalColorChanged().connect(
      sigc::mem_fun(*this, &ColorSequenceWidget::onFirstColorChange));
  _widgets.back()->show();

  _scrolledWindow.add(_box);
  _box.show();
  _frame.add(_scrolledWindow);
  _scrolledWindow.show();
  pack_start(_frame, true, true);
  _frame.show();
}

ColorSequenceWidget::~ColorSequenceWidget() = default;

void ColorSequenceWidget::OnGradient() {
  if (!_allEqual.get_active() && _widgets.size() > 2) {
    gradient_window_ = std::make_unique<GradientWindow>(_widgets.size());
    gradient_window_->set_modal(true);
    gradient_window_->set_transient_for(*_parent);
    gradient_window_->signal_hide().connect([&]() { OnGradientSelected(); });
    gradient_window_->show();
  }
}

void ColorSequenceWidget::OnGradientSelected() {
  if (gradient_window_->Result()) {
    std::vector<Color> colors = gradient_window_->GetColors();
    _widgets.front()->SetColor(colors.front());
    _widgets.back()->SetColor(colors.back());

    for (size_t i = 1; i < _widgets.size() - 1; ++i) {
      double floatIndex =
          static_cast<double>(i) * (colors.size() - 1) / (_widgets.size() - 1);
      const Color leftColor = colors[floor(floatIndex)];
      const Color rightColor = colors[floor(floatIndex) + 1];
      const double balance = floatIndex - floor(floatIndex);
      const unsigned red =
          (rightColor.Red() * balance + leftColor.Red() * (1.0 - balance));
      const unsigned green =
          (rightColor.Green() * balance + leftColor.Green() * (1.0 - balance));
      const unsigned blue =
          (rightColor.Blue() * balance + leftColor.Blue() * (1.0 - balance));
      _widgets[i]->SetColor(Color(red, green, blue));
    }
  }
  gradient_window_->close();
}

void ColorSequenceWidget::Shuffle() {
  std::vector<glight::theatre::ColorOrVariable> selection = GetSelection();
  std::shuffle(selection.begin(), selection.end(), std::random_device());
  SetSelection(selection);
}

void ColorSequenceWidget::LoadDefault() {
  Gtk::TreeIter active = _loadDefaultCombo.get_active();
  const std::string default_name =
      Glib::ustring((*active)[_listColumns._title]);
  const std::vector<theatre::Color> colors =
      theatre::GetDefaultColorSequence(default_name, _widgets.size());
  if (!colors.empty()) SetColors(colors);
}

void ColorSequenceWidget::OnOddEven() {
  if (_widgets.size() > 2) {
    _allEqual.set_active(false);
    const Color odd_color = _widgets[0]->GetColor();
    const Color even_color = _widgets[1]->GetColor();
    for (size_t i = 2; i != _widgets.size(); ++i) {
      _widgets[i]->SetColor(i % 2 == 0 ? odd_color : even_color);
    }
  }
}

}  // namespace glight::gui
