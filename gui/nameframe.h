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
		NameFrame(class Management &management, class ShowWindow& showWindow);
		~NameFrame();

		void SetNamedObject(class FolderObject &namedObject)
		{
			_namedObject = &namedObject;
			update();
		}
		FolderObject* GetNamedObject() const
		{
			return _namedObject;
		}
		void SetNoNamedObject()
		{
			_namedObject = 0;
			update();
		}
		sigc::signal<void>& SignalNameChange() { return _signalNameChange; }
		void ChangeManagement(class Management& management)
		{
			_management = &management;
		}
	private:
		void onButtonClicked();
		void update();

		class Management* _management;
		class ShowWindow& _showWindow;
		class FolderObject* _namedObject;

		Gtk::Entry _entry;
		Gtk::Label _label;
		Gtk::HButtonBox _buttonBox;
		Gtk::Button _button;
		sigc::signal<void> _signalNameChange;
};

#endif
