#include "effectpropertieswindow.h"
#include "showwindow.h"

#include "../libtheatre/effect.h"
#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"

#include <gtkmm/stock.h>

EffectPropertiesWindow::EffectPropertiesWindow(class Effect& effect, Management& management, ShowWindow& parentWindow) :
	PropertiesWindow(),
	
	_titleLabel("Effect " + effect.Name() + " (" + effect.TypeToName(effect.GetType()) + ")"),
	_connectionsFrame("Connections"),
	_propertiesFrame("Properties"),
	_addConnectionButton(Gtk::Stock::ADD),
	_removeConnectionButton(Gtk::Stock::REMOVE),
	
	_effect(&effect),
	_management(&management),
	_parentWindow(parentWindow)
{
	set_title("glight - " + effect.Name());
	set_size_request(650, 250);
	
	parentWindow.SignalChangeManagement().connect(sigc::mem_fun(*this, &EffectPropertiesWindow::onChangeManagement));
	parentWindow.SignalUpdateControllables().connect(sigc::mem_fun(*this, &EffectPropertiesWindow::onUpdateControllables));
	
	_controllablesMenu.SignalInputSelected().connect(sigc::mem_fun(*this, &EffectPropertiesWindow::onInputSelected));
	
	_topBox.pack_start(_titleLabel);
	
	_addConnectionButton.set_events(Gdk::BUTTON_PRESS_MASK);
	_addConnectionButton.signal_button_press_event().
		connect(sigc::mem_fun(*this, &EffectPropertiesWindow::onAddConnectionClicked), false);
	_connectionsButtonBox.pack_start(_addConnectionButton);

	_removeConnectionButton.signal_clicked().
		connect(sigc::mem_fun(*this, &EffectPropertiesWindow::onRemoveConnectionClicked));
	_removeConnectionButton.set_sensitive(false);
	_connectionsButtonBox.pack_start(_removeConnectionButton);

	_connectionsBox.pack_start(_connectionsButtonBox, false, false, 5);
	
	_connectionsListModel =
    Gtk::ListStore::create(_connectionsListColumns);

	_connectionsListView.set_model(_connectionsListModel);
	_connectionsListView.append_column("Connected control", _connectionsListColumns._title);
	_connectionsListView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &EffectPropertiesWindow::onSelectedConnectionChanged));
	_connectionsScrolledWindow.add(_connectionsListView);

	_connectionsScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_connectionsBox.pack_start(_connectionsScrolledWindow, true, true);
	_connectionsFrame.add(_connectionsBox);
	   _mainHBox.pack_start(_connectionsFrame);

	_propertiesFrame.add(_propertiesBox);
	
	   _mainHBox.pack_start(_propertiesFrame);
	
	_topBox.pack_start(_mainHBox);
	
	add(_topBox);
	
	fillConnectionsList();
	_propertySet = PropertySet::Make(effect);
	_propertiesBox.SetPropertySet(_propertySet.get());
	
	show_all_children();
}

FolderObject& EffectPropertiesWindow::GetObject()
{
	return *_effect;
}

void EffectPropertiesWindow::fillConnectionsList()
{
	_connectionsListModel->clear();

	std::lock_guard<std::mutex> lock(_management->Mutex());
	for(size_t index=0; index!=_effect->Connections().size(); ++index)
	{
		Gtk::TreeModel::iterator iter = _connectionsListModel->append();
		Gtk::TreeModel::Row row = *iter;
		row[_connectionsListColumns._title] = _effect->Connections()[index].first->Name();
		row[_connectionsListColumns._index] = index;
		row[_connectionsListColumns._inputIndex] = _effect->Connections()[index].second;
	}
}

void EffectPropertiesWindow::onSelectedConnectionChanged()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
		_connectionsListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	_removeConnectionButton.set_sensitive(bool(selected));
}

bool EffectPropertiesWindow::onAddConnectionClicked(GdkEventButton* event)
{
	_controllablesMenu.Popup(*_management, event);
	return true;
}

void EffectPropertiesWindow::onRemoveConnectionClicked()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
		_connectionsListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
		_effect->RemoveConnection((*selected)[_connectionsListColumns._index]);
	fillConnectionsList();
}

void EffectPropertiesWindow::onInputSelected(class PresetValue* preset)
{
	_effect->AddConnection(&preset->Controllable(), preset->InputIndex());
	fillConnectionsList();
}

void EffectPropertiesWindow::onUpdateControllables()
{
	if(_management->Contains(*_effect))
	{
		fillConnectionsList();
		_propertySet = PropertySet::Make(*_effect);
		_propertiesBox.SetPropertySet(_propertySet.get());
	}
	else 
		hide();
}
