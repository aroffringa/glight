#ifndef NAMEFRAME_H
#define NAMEFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>

/**
	@author Andre Offringa
*/
class NameFrame : public Gtk::HBox {
	public:
		NameFrame(class Management &management);
		~NameFrame();

		void SetNamedObject(class NamedObject &namedObject)
		{
			_namedObject = &namedObject;
			update();
		}
		void SetNoNamedObject()
		{
			_namedObject = 0;
			update();
		}
		sigc::signal<void>& SignalNameChange() { return _signalNameChange; }
	private:
		void onButtonClicked();
		void update();

		class Management &_management;
		class NamedObject *_namedObject;

		Gtk::Entry _entry;
		Gtk::Label _label;
		Gtk::HButtonBox _buttonBox;
		Gtk::Button _button;
		sigc::signal<void> _signalNameChange;
};

#endif
