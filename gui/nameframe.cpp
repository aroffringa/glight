#include "nameframe.h"

#include "../libtheatre/management.h"
#include "../libtheatre/namedobject.h"

#include <gtkmm/stock.h>

NameFrame::NameFrame(Management &management) :
	_management(management),
	_namedObject(0),
	_label("Name:"),
	_button(Gtk::Stock::APPLY)
{
	pack_start(_label, false, false, 2);
	_label.show();

	pack_start(_entry, true, true, 2);
	_entry.show();

	_button.signal_clicked().connect(sigc::mem_fun(*this, &NameFrame::onButtonClicked));
	_buttonBox.pack_start(_button, false, false, 0);
	_button.show();

	pack_start(_buttonBox, false, false, 2);
	_buttonBox.show();

	update();
}

NameFrame::~NameFrame()
{
}

void NameFrame::update()
{
	if(_namedObject == 0)
	{
		_entry.set_text("");
		set_sensitive(false);
	} else {
		_entry.set_text(_namedObject->Name());
		set_sensitive(true);
	}
}

void NameFrame::onButtonClicked()
{
	if(_namedObject != 0)
	{
		std::unique_lock<std::mutex> lock(_management.Mutex());
		_namedObject->SetName(_entry.get_text());
		lock.unlock();

		_signalNameChange();
	}
}
