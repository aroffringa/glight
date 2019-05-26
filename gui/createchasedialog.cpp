
#include "createchasedialog.h"

#include <gtkmm/stock.h>

#include "showwindow.h"

#include "../libtheatre/chase.h"
#include "../libtheatre/folder.h"
#include "../libtheatre/management.h"
#include "../libtheatre/presetvalue.h"
#include "../libtheatre/presetcollection.h"
#include "../libtheatre/sequence.h"

CreateChaseDialog::CreateChaseDialog(Management& management, ShowWindow& parentWindow) :
	Dialog("Create chase", true),
	_listFrame("Object list"),
	_list(management, parentWindow),
	_newChaseFrame("Chase objects"),
	_addObjectToChaseButton(Gtk::Stock::ADD),
	_clearChaseButton("Clear"),
	_management(&management),
	_parentWindow(parentWindow)
{
	_parentWindow.SignalChangeManagement().connect(sigc::mem_fun(*this, &CreateChaseDialog::changeManagement));
	
	set_size_request(600, 400);
	
	initListPart();
	initNewSequencePart();

	_paned.pack1(_listFrame);
	_paned.pack2(_newChaseFrame);
	get_content_area()->pack_start(_paned);
	
	add_button("Cancel", Gtk::RESPONSE_CANCEL);
	_makeChaseButton = add_button("Make chase", Gtk::RESPONSE_OK);
	_makeChaseButton->signal_clicked().connect(sigc::mem_fun(*this, &CreateChaseDialog::onCreateChaseButtonClicked));
	_makeChaseButton->set_sensitive(false);
	
	show_all_children();
	
}

void CreateChaseDialog::initListPart()
{
	_list.SignalSelectionChange().connect(sigc::mem_fun(this, &CreateChaseDialog::onSelectedObjectChanged));
	_list.SetDisplayType(ObjectList::OnlyPresetCollections);
	
	_listVBox.pack_start(_list);
	_listFrame.add(_listVBox);
}

void CreateChaseDialog::initNewSequencePart()
{
	_addObjectToChaseButton.set_sensitive(false);
	_addObjectToChaseButton.signal_clicked().
		connect(sigc::mem_fun(*this, &CreateChaseDialog::onAddObjectToChaseButtonClicked));
	_newChaseButtonBox.pack_start(_addObjectToChaseButton);

	_clearChaseButton.signal_clicked().
		connect(sigc::mem_fun(*this, &CreateChaseDialog::onClearSequenceButtonClicked));
	_newChaseButtonBox.pack_start(_clearChaseButton);

	_newChaseBox.pack_start(_newChaseButtonBox, false, false, 5);
	
	_newChaseListModel =
    Gtk::ListStore::create(_newChaseListColumns);

	_newChaseListView.set_model(_newChaseListModel);
	_newChaseListView.append_column("Chase object list", _newChaseListColumns._title);
	_newChaseScrolledWindow.add(_newChaseListView);

	_newChaseScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_newChaseBox.pack_start(_newChaseScrolledWindow);

	_newChaseFrame.add(_newChaseBox);
}

void CreateChaseDialog::onAddObjectToChaseButtonClicked()
{
	FolderObject* selectedObj = _list.SelectedObject();
	if(selectedObj)
	{
		Controllable* object = dynamic_cast<Controllable*>(selectedObj);
		if(object)
		{
			Gtk::TreeModel::iterator newRow = _newChaseListModel->append();
			std::lock_guard<std::mutex> lock(_management->Mutex());
			(*newRow)[_newChaseListColumns._title] = object->Name();
			(*newRow)[_newChaseListColumns._controllable] = object;
			_makeChaseButton->set_sensitive(true);
		}
	}
}

void CreateChaseDialog::onClearSequenceButtonClicked()
{
	_newChaseListModel->clear();
	_makeChaseButton->set_sensitive(false);
}

void CreateChaseDialog::onCreateChaseButtonClicked()
{
	if(!_newChaseListModel->children().empty())
	{
		// Determine folder
		Folder& folder = _list.SelectedFolder();
		std::unique_lock<std::mutex> lock(_management->Mutex());
		
		Chase& chase = _management->AddChase();
		_management->AddPreset(chase, 0);
		std::stringstream s;
		s << "#" << _management->Controllables().size();
		chase.SetName(s.str());
		folder.Add(chase);

		Sequence& sequence = chase.Sequence();
		Gtk::TreeModel::Children children = _newChaseListModel->children();
		for(Gtk::TreeModel::Children::const_iterator i=children.begin();
			i != children.end() ; ++i)
		{
			Controllable *object = (*i)[_newChaseListColumns._controllable];
			sequence.Add(object);
		}

		lock.unlock();

		_parentWindow.EmitUpdate();
		_newChaseListModel->clear();
		_makeChaseButton->set_sensitive(false);
		_parentWindow.MakeChaseTabActive(chase);
	}
}

void CreateChaseDialog::onSelectedObjectChanged()
{
	if(_delayUpdates.IsFirst())
	{
		FolderObject* selectedObj = _list.SelectedObject();
		PresetCollection *preset = dynamic_cast<PresetCollection*>(selectedObj);
		if(preset)
			_addObjectToChaseButton.set_sensitive(true);
		else
			_addObjectToChaseButton.set_sensitive(false);
	}
}
