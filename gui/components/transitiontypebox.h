#ifndef GUI_TRANSITION_TYPE_BOX_H_
#define GUI_TRANSITION_TYPE_BOX_H_

#include "../../theatre/transition.h"

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

namespace glight::gui {

class TransitionTypeBox : public Gtk::VBox {
 public:
  TransitionTypeBox(
      theatre::TransitionType value = theatre::TransitionType::Fade) {
    _topBox.pack_start(_label);

    Gtk::RadioButtonGroup transTypeGroup;
    _noneRB.set_group(transTypeGroup);
    _noneRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::None); });
    _topBox.pack_start(_noneRB);

    _fadeRB.set_group(transTypeGroup);
    _fadeRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::Fade); });
    _topBox.pack_start(_fadeRB);

    _fadeThroughBlackRB.set_group(transTypeGroup);
    _fadeThroughBlackRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::FadeThroughBlack); });
    _topBox.pack_start(_fadeThroughBlackRB);

    _fadeThroughFullRB.set_group(transTypeGroup);
    _fadeThroughFullRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::FadeThroughFull); });
    _topBox.pack_start(_fadeThroughFullRB);

    _glowFadeRB.set_group(transTypeGroup);
    _glowFadeRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::GlowFade); });
    _centreBox.pack_start(_glowFadeRB);

    _steppedRB.set_group(transTypeGroup);
    _steppedRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::Stepped); });
    _centreBox.pack_start(_steppedRB);

    _randomRB.set_group(transTypeGroup);
    _randomRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::Random); });
    _centreBox.pack_start(_randomRB);

    _erraticRB.set_group(transTypeGroup);
    _erraticRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::Erratic); });
    _centreBox.pack_start(_erraticRB);

    _blackRB.set_group(transTypeGroup);
    _blackRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::Black); });
    _bottomBox.pack_start(_blackRB);

    _fadeFromBlackRB.set_group(transTypeGroup);
    _fadeFromBlackRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::FadeFromBlack); });
    _bottomBox.pack_start(_fadeFromBlackRB);

    _fadeToBlackRB.set_group(transTypeGroup);
    _fadeToBlackRB.signal_clicked().connect(
        [&]() { Change(theatre::TransitionType::FadeToBlack); });
    _bottomBox.pack_start(_fadeToBlackRB);

    pack_start(_topBox);
    pack_start(_centreBox);
    pack_end(_bottomBox);

    Set(value);
  }

  sigc::signal<void(theatre::TransitionType)> &SignalChanged() {
    return _signalChanged;
  }

  void Set(theatre::TransitionType type) {
    using theatre::TransitionType;
    switch (type) {
      case TransitionType::None:
        _noneRB.set_active();
        break;
      case TransitionType::Fade:
        _fadeRB.set_active();
        break;
      case TransitionType::FadeThroughBlack:
        _fadeThroughBlackRB.set_active();
        break;
      case TransitionType::FadeThroughFull:
        _fadeThroughFullRB.set_active();
        break;
      case TransitionType::GlowFade:
        _glowFadeRB.set_active();
        break;
      case TransitionType::Stepped:
        _steppedRB.set_active();
        break;
      case TransitionType::Random:
        _randomRB.set_active();
        break;
      case TransitionType::Erratic:
        _erraticRB.set_active();
        break;
      case TransitionType::Black:
        _blackRB.set_active();
        break;
      case TransitionType::FadeFromBlack:
        _fadeFromBlackRB.set_active();
        break;
      case TransitionType::FadeToBlack:
        _fadeToBlackRB.set_active();
        break;
    }
  }

  theatre::TransitionType Get() const { return _value; }

 private:
  void Change(theatre::TransitionType type) {
    _value = type;
    _signalChanged(type);
  }

  Gtk::Label _label{"Type:"};
  Gtk::HBox _topBox, _centreBox, _bottomBox;
  Gtk::RadioButton _noneRB{"None"};
  Gtk::RadioButton _fadeRB{"Fade"};
  Gtk::RadioButton _fadeThroughBlackRB{"Through black"};
  Gtk::RadioButton _fadeThroughFullRB{"Through full"};
  Gtk::RadioButton _glowFadeRB{"Glow fade"};
  Gtk::RadioButton _steppedRB{"Stepped"};
  Gtk::RadioButton _randomRB{"Random"};
  Gtk::RadioButton _erraticRB{"Erratic"};
  Gtk::RadioButton _blackRB{"Black out"};
  Gtk::RadioButton _fadeFromBlackRB{"From black"};
  Gtk::RadioButton _fadeToBlackRB{"To black"};
  sigc::signal<void(theatre::TransitionType)> _signalChanged;
  theatre::TransitionType _value;
};

}  // namespace glight::gui

#endif
