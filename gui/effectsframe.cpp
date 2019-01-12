#include "effectsframe.h"

#include "showwindow.h"

#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"

#include "../libtheatre/effects/thresholdeffect.h"

#include <gtkmm/stock.h>

EffectsFrame::EffectsFrame(Management &management, ShowWindow &parentWindow) :
	_effectsFrame("Effects"),
	_newEffectButton(Gtk::Stock::NEW), 
	_deleteEffectButton(Gtk::Stock::DELETE),
	_nameFrame(management),
	_connectionsFrame("Connections"),
	_propertiesFrame("Properties"),
	_addConnectionButton(Gtk::Stock::ADD),
	_removeConnectionButton(Gtk::Stock::REMOVE),
	_management(&management),
	_parentWindow(parentWindow)
{
	parentWindow.SignalUpdateControllables().connect(sigc::mem_fun(*this, &EffectsFrame::fillEffectsList));
	
	initEffectsPart();
	initPropertiesPart();

	pack1(_effectsFrame);
	pack2(_propertiesHBox);
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
	_controllablesMenu.SignalControllableSelected().connect(sigc::mem_fun(*this, &EffectsFrame::onControllableSelected));
	
	_addConnectionButton.set_events(Gdk::BUTTON_PRESS_MASK);
	_addConnectionButton.signal_button_press_event().
		connect(sigc::mem_fun(*this, &EffectsFrame::onAddConnectionClicked), false);
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
	_connectionsListView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &EffectsFrame::onSelectedConnectionChanged));
	_connectionsScrolledWindow.add(_connectionsListView);

	_connectionsScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_connectionsBox.pack_start(_connectionsScrolledWindow, true, true);
	_connectionsFrame.add(_connectionsBox);
	_connectionsFrame.set_sensitive(false);
	_propertiesHBox.pack_start(_connectionsFrame);

	_propertiesHBox.pack_start(_connectionsBox, true, true, 5);

	_propertiesFrame.add(_propertiesBox);
	
	_propertiesFrame.set_sensitive(false);
	_propertiesHBox.pack_start(_propertiesFrame);
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

Effect* EffectsFrame::getSelectedEffect()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
		_effectsListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
		return (*selected)[_effectsListColumns._effect];
	else
		return nullptr;
}

void EffectsFrame::onSelectedEffectChanged()
{
	if(_delayUpdates.IsFirst())
	{
		Effect* effect = getSelectedEffect();
		if(effect)
		{
			_nameFrame.SetNamedObject(*effect);
			_connectionsFrame.set_sensitive(true);
			_propertiesFrame.set_sensitive(true);
			fillProperties(*effect);
		}
		else
		{
			_nameFrame.SetNoNamedObject();
			_connectionsFrame.set_sensitive(false);
			_propertiesFrame.set_sensitive(false);
		}
	}
}

void EffectsFrame::fillProperties(Effect& effect)
{
	fillConnectionsList(effect);
	
	ThresholdEffect *threshold = dynamic_cast<ThresholdEffect*>(&effect);
	if(threshold != nullptr)
	{
		_thresholdLowerStart.set_text(std::to_string(100.0*threshold->LowerStartLimit()/ControlValue::MaxUInt()));
		_thresholdLowerEnd.set_text(std::to_string(100.0*threshold->LowerEndLimit()/ControlValue::MaxUInt()));
		_thresholdUpperStart.set_text(std::to_string(100.0*threshold->UpperStartLimit()/ControlValue::MaxUInt()));
		_thresholdUpperEnd.set_text(std::to_string(100.0*threshold->UpperEndLimit()/ControlValue::MaxUInt()));
	}
}

void EffectsFrame::onApplyPropertiesClicked()
{
	Effect* effect = getSelectedEffect();
	if(effect)
	{
		ThresholdEffect *threshold = dynamic_cast<ThresholdEffect*>(effect);
		if(threshold != nullptr)
		{
			threshold->SetLowerStartLimit(unsigned(std::atof(_thresholdLowerStart.get_text().c_str())*ControlValue::MaxUInt()/100.0));
			threshold->SetLowerEndLimit(unsigned(std::atof(_thresholdLowerEnd.get_text().c_str())*ControlValue::MaxUInt()/100.0));
			threshold->SetUpperStartLimit(unsigned(std::atof(_thresholdUpperStart.get_text().c_str())*ControlValue::MaxUInt()/100.0));
			threshold->SetUpperEndLimit(unsigned(std::atof(_thresholdUpperEnd.get_text().c_str())*ControlValue::MaxUInt()/100.0));
		}
	}
}

void EffectsFrame::fillConnectionsList(Effect& effect)
{
	_connectionsListModel->clear();

	std::lock_guard<std::mutex> lock(_management->Mutex());
	const std::vector<Controllable*>&
		connections = effect.Connections();
	for(size_t index=0; index!=connections.size(); ++index)
	{
		Gtk::TreeModel::iterator iter = _connectionsListModel->append();
		Gtk::TreeModel::Row row = *iter;
		row[_connectionsListColumns._title] = connections[index]->Name();
		row[_connectionsListColumns._index] = index;
	}
}

void EffectsFrame::onSelectedConnectionChanged()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
		_connectionsListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	_removeConnectionButton.set_sensitive(bool(selected));
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
	Effect* effect = getSelectedEffect();
	if(effect)
	{
		Glib::RefPtr<Gtk::TreeSelection> selection =
			_connectionsListView.get_selection();
		Gtk::TreeModel::iterator selected = selection->get_selected();
		if(selected)
			effect->RemoveConnection((*selected)[_connectionsListColumns._index]);
		fillConnectionsList(*effect);
	}
}

void EffectsFrame::onControllableSelected(class PresetValue* preset)
{
	Effect* effect = getSelectedEffect();
	if(effect)
	{
		effect->AddConnection(&preset->Controllable());
		fillConnectionsList(*effect);
	}
}
