#ifndef COLOR_SEQUENCE_WIDGET_H
#define COLOR_SEQUENCE_WIDGET_H

#include "colorselectwidget.h"

#include "theatre/color.h"
#include "theatre/forwards.h"

#include <gtkmm/box.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>

#include <vector>

namespace glight::gui {

using theatre::Color;
using theatre::ColorOrVariable;

class EventTransmitter;
class GradientWindow;

class ColorSequenceWidget : public Gtk::VBox {
 public:
  ColorSequenceWidget(Gtk::Window *parent, bool showGradientButton = true,
                      bool showShuffleButton = true);

  ~ColorSequenceWidget();

  void SetColors(const std::vector<Color> &colors) {
    std::vector<ColorOrVariable> values;
    values.reserve(colors.size());
    for (const Color &c : colors) {
      values.emplace_back(c);
    }
    SetSelection(values);
  }

  void SetSelection(const std::vector<ColorOrVariable> &values) {
    if (_maxCount < values.size()) _maxCount = 0;
    if (values.size() < _minCount) _minCount = values.size();
    _allEqual.set_active(false);
    _widgets.clear();
    for (size_t i = 0; i != values.size(); ++i) {
      _widgets.emplace_back(std::make_unique<ColorSelectWidget>(_parent, true));
      if (i == 0) {
        _widgets.back()->SignalColorChanged().connect(
            sigc::mem_fun(*this, &ColorSequenceWidget::onFirstColorChange));
      }
      _widgets.back()->SetSelection(values[i]);
      _widgets.back()->SetAllowVariables(allow_variables_);
      _box.pack_start(*_widgets.back(), true, false);
      _widgets.back()->show();
    }
  }

  std::vector<Color> GetColors() const {
    std::vector<Color> result;
    result.reserve(_widgets.size());
    for (const std::unique_ptr<ColorSelectWidget> &w : _widgets)
      result.emplace_back(w->GetColor());
    return result;
  }

  std::vector<ColorOrVariable> GetSelection() const {
    std::vector<ColorOrVariable> result;
    result.reserve(_widgets.size());
    for (const std::unique_ptr<ColorSelectWidget> &w : _widgets)
      result.emplace_back(w->GetSelection());
    return result;
  }

  void SetMinCount(size_t minCount) {
    if (_maxCount < minCount && _maxCount != 0) SetMaxCount(minCount);
    while (_widgets.size() < minCount) onIncreaseColors();
    _minCount = minCount;
    updateSensitivities();
  }

  void SetMaxCount(size_t maxCount) {
    if (_minCount > maxCount && maxCount != 0) SetMinCount(maxCount);
    _maxCount = maxCount;
    if (_maxCount != 0) {
      if (_widgets.size() > _maxCount) {
        _widgets.resize(_maxCount);
      }
    }
    updateSensitivities();
  }

  void SetAllowVariables(bool allow_variables) {
    allow_variables_ = allow_variables;
  }

 private:
  Gtk::Frame _frame;
  Gtk::ScrolledWindow _scrolledWindow;
  Gtk::VBox _box;
  Gtk::CheckButton _allEqual;
  std::vector<std::unique_ptr<ColorSelectWidget>> _widgets;
  Gtk::Box _buttonBox;
  Gtk::Button _plusButton, _gradientButton, _shuffleButton, _minButton;
  Gtk::HBox _loadDefaultBox;
  Gtk::ComboBox _loadDefaultCombo;
  Gtk::Button _loadDefaultButton;
  Glib::RefPtr<Gtk::ListStore> _loadDefaultList;
  struct ListColumns : public Gtk::TreeModelColumnRecord {
    ListColumns() { add(_title); }
    Gtk::TreeModelColumn<Glib::ustring> _title;
  } _listColumns;
  Gtk::Window *_parent;
  std::unique_ptr<GradientWindow> gradient_window_;
  size_t _minCount, _maxCount;
  bool allow_variables_ = true;

  void LoadDefault();

  void onDecreaseColors() {
    if (_widgets.size() > _minCount) {
      _widgets.pop_back();
      updateSensitivities();
    }
  }
  void onIncreaseColors() {
    if (_maxCount == 0 || _widgets.size() < _maxCount) {
      _widgets.emplace_back(std::make_unique<ColorSelectWidget>(_parent, true));
      _widgets.back()->SetAllowVariables(allow_variables_);
      if (_allEqual.get_active()) {
        _widgets.back()->SetSelection(_widgets.front()->GetSelection());
        _widgets.back()->set_sensitive(false);
      }
      updateSensitivities();
      _box.pack_start(*_widgets.back(), true, false);
      _widgets.back()->show();
      queue_resize();
    }
  }

  void updateSensitivities() {
    _minButton.set_sensitive(_widgets.size() > _minCount);
    _gradientButton.set_sensitive(_widgets.size() > 2);
    _shuffleButton.set_sensitive(_widgets.size() > 1);
    _plusButton.set_sensitive(_maxCount == 0 || _widgets.size() < _maxCount);
  }

  void OnGradient();
  void OnGradientSelected();
  void Shuffle();

  void onFirstColorChange() {
    if (_allEqual.get_active()) {
      const ColorOrVariable value = _widgets.front()->GetSelection();
      for (size_t i = 1; i != _widgets.size(); ++i) {
        _widgets[i]->SetSelection(value);
        _widgets[i]->set_sensitive(false);
      }
    }
  }
  void onToggleEqual() {
    if (_allEqual.get_active()) {
      const ColorOrVariable value = _widgets.front()->GetColor();
      for (size_t i = 1; i != _widgets.size(); ++i) {
        _widgets[i]->SetSelection(value);
        _widgets[i]->set_sensitive(false);
      }
    } else {
      for (size_t i = 1; i != _widgets.size(); ++i) {
        _widgets[i]->set_sensitive(true);
      }
    }
  }
};

}  // namespace glight::gui

#endif
