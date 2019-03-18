#include "nameframe.h"

#include "showwindow.h"

#include "../libtheatre/folder.h"
#include "../libtheatre/management.h"
#include "../libtheatre/namedobject.h"

#include <gtkmm/stock.h>

NameFrame::NameFrame(Management& management, ShowWindow& showWindow) :
	_management(&management),
	_showWindow(showWindow),
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
	if(_namedObject == nullptr)
	{
		_entry.set_text("");
		set_sensitive(false);
	}
	else if(_namedObject == &_management->RootFolder())
	{
		_entry.set_text("Root");
		set_sensitive(false);
	} else {
		_entry.set_text(_namedObject->Name());
		set_sensitive(true);
	}
}

void NameFrame::onButtonClicked()
{
	if(_namedObject != nullptr)
	{
		std::unique_lock<std::mutex> lock(_management->Mutex());
		_namedObject->SetName(_entry.get_text());
		lock.unlock();

		_showWindow.EmitUpdate();
		_signalNameChange();
	}
}
