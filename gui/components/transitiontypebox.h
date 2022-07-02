#ifndef TRANSITION_TYPE_BOX_H
#define TRANSITION_TYPE_BOX_H

#include "../../theatre/transition.h"

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

class TransitionTypeBox : public Gtk::VBox {
 public:
  TransitionTypeBox(TransitionType value = TransitionType::Fade)
      : _label("Type:"),
        _noneRB("None"),
        _fadeRB("Fade"),
        _fadeThroughBlackRB("Through black"),
        _erraticRB("Erratic"),
        _blackRB("Black out"),
        _fadeFromBlackRB("From black"),
        _fadeToBlackRB("To black") {
    _topBox.pack_start(_label);

    Gtk::RadioButtonGroup transTypeGroup;
    _noneRB.set_group(transTypeGroup);
    _noneRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(TransitionType::None); });
    _topBox.pack_start(_noneRB);

    _fadeRB.set_group(transTypeGroup);
    _fadeRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(TransitionType::Fade); });
    _topBox.pack_start(_fadeRB);

    _fadeThroughBlackRB.set_group(transTypeGroup);
    _fadeThroughBlackRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(TransitionType::FadeThroughBlack); });
    _topBox.pack_start(_fadeThroughBlackRB);

    _erraticRB.set_group(transTypeGroup);
    _erraticRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(TransitionType::Erratic); });
    _topBox.pack_start(_erraticRB);

    _blackRB.set_group(transTypeGroup);
    _blackRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(TransitionType::Black); });
    _bottomBox.pack_start(_blackRB);

    _fadeFromBlackRB.set_group(transTypeGroup);
    _fadeFromBlackRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(TransitionType::FadeFromBlack); });
    _bottomBox.pack_start(_fadeFromBlackRB);

    _fadeToBlackRB.set_group(transTypeGroup);
    _fadeToBlackRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(TransitionType::FadeToBlack); });
    _bottomBox.pack_start(_fadeToBlackRB);

    pack_start(_topBox);
    pack_end(_bottomBox);

    Set(value);
  }

  sigc::signal<void(TransitionType)> &SignalChanged() { return _signalChanged; }

  void Set(TransitionType type) {
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

 private:
  Gtk::Label _label;
  Gtk::HBox _topBox, _bottomBox;
  Gtk::RadioButton _noneRB, _fadeRB, _fadeThroughBlackRB, _erraticRB, _blackRB,
      _fadeFromBlackRB, _fadeToBlackRB;
  sigc::signal<void(TransitionType)> _signalChanged;
};

#endif
