#include "effectsframe.h"

#include "showwindow.h"

#include "../libtheatre/management.h"
#include "../libtheatre/effects/thresholdeffect.h"

#include <gtkmm/stock.h>

EffectsFrame::EffectsFrame(Management &management, ShowWindow &parentWindow) :
	_effectsFrame("Effects"),
	_connectionsFrame("Connections"),
	_newEffectButton(Gtk::Stock::NEW), 
	_deleteEffectButton(Gtk::Stock::DELETE), 
	_addConnectionButton(Gtk::Stock::ADD),
	_removeConnectionButton(Gtk::Stock::REMOVE),
	_management(&management),
	_parentWindow(parentWindow),
	_nameFrame(management)
{
	parentWindow.SignalUpdateControllables().connect(sigc::mem_fun(*this, &EffectsFrame::fillEffectsList));
	
	initEffectsPart();
	initPropertiesPart();

	pack1(_effectsFrame);
	pack2(_connectionsFrame);
	show_all_children();
}

void EffectsFrame::initEffectsPart()
{
	_newEffectButton.signal_clicked().
		connect(sigc::mem_fun(*this, &EffectsFrame::onNewEffectClicked));
	_effectsButtonBox.pack_start(_newEffectButton);

	_deleteEffectButton.signal_clicked().
		connect(sigc::mem_fun(*this, &EffectsFrame::onDeleteEffectClicked));
	_effectsButtonBox.pack_start(_deleteEffectButton);

	_effectsHBox.pack_start(_effectsButtonBox, false, false, 5);

	_effectsListModel =
    Gtk::ListStore::create(_effectsListColumns);

	_effectsListView.set_model(_effectsListModel);
	_effectsListView.append_column("Effect", _effectsListColumns._title);
	_effectsListView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &EffectsFrame::onSelectedEffectChanged));
	fillEffectsList();
	_effectsScrolledWindow.add(_effectsListView);

	_effectsScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_effectsHBox.pack_start(_effectsScrolledWindow);

	_effectsVBox.pack_start(_effectsHBox);

	_nameFrame.SignalNameChange().connect(sigc::mem_fun(*this, &EffectsFrame::onNameChange));
	_effectsVBox.pack_start(_nameFrame, false, false, 2);

	_effectsFrame.add(_effectsVBox);
}

void EffectsFrame::initPropertiesPart()
{
	_addConnectionButton.set_events(Gdk::BUTTON_PRESS_MASK);
	_addConnectionButton.signal_button_press_event().
		connect(sigc::mem_fun(*this, &EffectsFrame::onAddConnectionClicked));
	_addConnectionButton.set_sensitive(false);
	_connectionsButtonBox.pack_start(_addConnectionButton);

	_removeConnectionButton.signal_clicked().
		connect(sigc::mem_fun(*this, &EffectsFrame::onRemoveConnectionClicked));
	_removeConnectionButton.set_sensitive(false);
	_connectionsButtonBox.pack_start(_removeConnectionButton);

	_connectionsBox.pack_start(_connectionsButtonBox, false, false, 5);
	
	_connectionsListModel =
    Gtk::ListStore::create(_connectionsListColumns);

	_connectionsListView.set_model(_connectionsListModel);
	_connectionsListView.append_column("Connected control", _connectionsListColumns._title);
	_connectionsScrolledWindow.add(_connectionsListView);

	_connectionsScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_connectionsBox.pack_start(_connectionsScrolledWindow);

	_connectionsFrame.add(_connectionsBox);
}

void EffectsFrame::fillEffectsList()
{
	AvoidRecursion::Token token(_delayUpdates);
	_effectsListModel->clear();

	std::lock_guard<std::mutex> lock(_management->Mutex());
	const std::vector<std::unique_ptr<Effect>>&
		effects = _management->Effects();
	for(const std::unique_ptr<Effect>& effect : effects)
	{
		Gtk::TreeModel::iterator iter = _effectsListModel->append();
		Gtk::TreeModel::Row row = *iter;
		row[_effectsListColumns._title] = effect->Name();
		row[_effectsListColumns._effect] = effect.get();
	}
}

void EffectsFrame::onSelectedEffectChanged()
{
}

void EffectsFrame::onNewEffectClicked()
{
	std::unique_ptr<Effect> effect(new ThresholdEffect());
	effect->SetName("Threshold");
	Effect* added = &_management->AddEffect(std::move(effect));
	for(EffectControl* ec : added->Controls())
		_management->AddPreset(*ec);
	_parentWindow.EmitUpdate();
}

void EffectsFrame::onDeleteEffectClicked()
{
}

bool EffectsFrame::onAddConnectionClicked(GdkEventButton* event)
{
	_controllablesMenu.Popup(*_management, event);
	return true;
}

void EffectsFrame::onRemoveConnectionClicked()
{
}
