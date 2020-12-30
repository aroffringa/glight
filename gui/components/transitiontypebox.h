#ifndef TRANSITION_TYPE_BOX_H
#define TRANSITION_TYPE_BOX_H

#include "../../theatre/transition.h"

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>

class TransitionTypeBox : public Gtk::VBox {
 public:
  TransitionTypeBox(enum Transition::Type value = Transition::Fade)
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
        [&]() { _signalChanged.emit(Transition::None); });
    _topBox.pack_start(_noneRB);

    _fadeRB.set_group(transTypeGroup);
    _fadeRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(Transition::Fade); });
    _topBox.pack_start(_fadeRB);

    _fadeThroughBlackRB.set_group(transTypeGroup);
    _fadeThroughBlackRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(Transition::FadeThroughBlack); });
    _topBox.pack_start(_fadeThroughBlackRB);

    _erraticRB.set_group(transTypeGroup);
    _erraticRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(Transition::Erratic); });
    _topBox.pack_start(_erraticRB);

    _blackRB.set_group(transTypeGroup);
    _blackRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(Transition::Black); });
    _bottomBox.pack_start(_blackRB);

    _fadeFromBlackRB.set_group(transTypeGroup);
    _fadeFromBlackRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(Transition::FadeFromBlack); });
    _bottomBox.pack_start(_fadeFromBlackRB);

    _fadeToBlackRB.set_group(transTypeGroup);
    _fadeToBlackRB.signal_clicked().connect(
        [&]() { _signalChanged.emit(Transition::FadeToBlack); });
    _bottomBox.pack_start(_fadeToBlackRB);

    pack_start(_topBox);
    pack_end(_bottomBox);

    Set(value);
  }

  sigc::signal<void(enum Transition::Type)> &SignalChanged() {
    return _signalChanged;
  }

  void Set(enum Transition::Type type) {
    switch (type) {
      case Transition::None:
        _noneRB.set_active();
        break;
      case Transition::Fade:
        _fadeRB.set_active();
        break;
      case Transition::FadeThroughBlack:
        _fadeThroughBlackRB.set_active();
        break;
      case Transition::Erratic:
        _erraticRB.set_active();
        break;
      case Transition::Black:
        _blackRB.set_active();
        break;
      case Transition::FadeFromBlack:
        _fadeFromBlackRB.set_active();
        break;
      case Transition::FadeToBlack:
        _fadeToBlackRB.set_active();
        break;
    }
  }

 private:
  Gtk::Label _label;
  Gtk::HBox _topBox, _bottomBox;
  Gtk::RadioButton _noneRB, _fadeRB, _fadeThroughBlackRB, _erraticRB, _blackRB,
      _fadeFromBlackRB, _fadeToBlackRB;
  sigc::signal<void(enum Transition::Type)> _signalChanged;
};

#endif
