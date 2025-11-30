#ifndef COLOR_SEQUENCE_WIDGET_H
#define COLOR_SEQUENCE_WIDGET_H

#include "colorselectwidget.h"

#include "theatre/color.h"
#include "theatre/forwards.h"

#include "system/optionalnumber.h"

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

class ColorSequenceWidget : public Gtk::Box {
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

  void SetSelection(const std::vector<ColorOrVariable> &values);

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
    if (max_count_ < minCount && max_count_ != 0) SetMaxCount(minCount);
    while (_widgets.size() < minCount) OnIncreaseColors();
    min_count_ = minCount;
    updateSensitivities();
  }

  void SetMaxCount(size_t maxCount) {
    if (min_count_ > maxCount && maxCount != 0) SetMinCount(maxCount);
    max_count_ = maxCount;
    if (max_count_ != 0) {
      while (_widgets.size() > max_count_ && _widgets.size() > min_count_) {
        OnDecreaseColors();
      }
    }
  }

  void SetAllowVariables(bool allow_variables) {
    allow_variables_ = allow_variables;
    for (std::unique_ptr<ColorSelectWidget> &w : _widgets) {
      w->SetAllowVariables(allow_variables);
    }
  }

 private:
  Gtk::Frame _frame{"Colors"};
  Gtk::ScrolledWindow _scrolledWindow;
  Gtk::Box _box;
  Gtk::Box repeat_box_;
  Gtk::Label repeat_label_{"Repeat color:"};
  Gtk::ComboBox repeat_combo_;
  Glib::RefPtr<Gtk::ListStore> repeat_list_;
  struct RepeatColumns : public Gtk::TreeModelColumnRecord {
    RepeatColumns() { add(description_); }
    Gtk::TreeModelColumn<Glib::ustring> description_;
  } repeat_columns_;
  std::vector<std::unique_ptr<ColorSelectWidget>> _widgets;
  Gtk::Box _buttonBox;
  Gtk::Button _plusButton{"+"};
  Gtk::Button _gradientButton{"Gradient"};
  Gtk::Button _shuffleButton{"Shuffle"};
  Gtk::Button _minButton{"-"};
  Gtk::Box _loadDefaultBox;
  Gtk::ComboBox _loadDefaultCombo;
  Gtk::Button _loadDefaultButton{"Load"};
  Glib::RefPtr<Gtk::ListStore> _loadDefaultList;
  struct ListColumns : public Gtk::TreeModelColumnRecord {
    ListColumns() { add(_title); }
    Gtk::TreeModelColumn<Glib::ustring> _title;
  } _listColumns;
  Gtk::Window *_parent;
  std::unique_ptr<GradientWindow> gradient_window_;
  size_t min_count_ = 1;
  size_t max_count_ = 0;
  bool allow_variables_ = true;

  void LoadDefault();

  void OnDecreaseColors();
  void OnIncreaseColors();

  void updateSensitivities() {
    _minButton.set_sensitive(_widgets.size() > min_count_);
    _gradientButton.set_sensitive(_widgets.size() > 2);
    _shuffleButton.set_sensitive(_widgets.size() > 1);
    _plusButton.set_sensitive(max_count_ == 0 || _widgets.size() < max_count_);
  }

  void OnGradient();
  void OnGradientSelected();
  void OnOddEven();
  void Shuffle();
  system::OptionalNumber<size_t> RepeatCount() const;

  void OnColorChange(size_t index);
  void OnChangeRepeat();
};

}  // namespace glight::gui

#endif
