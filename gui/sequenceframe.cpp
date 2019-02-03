#include "sequenceframe.h"

#include "../libtheatre/chase.h"
#include "../libtheatre/folder.h"
#include "../libtheatre/sequence.h"

#include "showwindow.h"

SequenceFrame::SequenceFrame(Management &management, ShowWindow &parentWindow) :
	_sequenceFrame("All sequences"),
	_sequenceList(management, parentWindow),
	_createChaseButton("Create chase"),
	_nameFrame(management),
	_management(&management),
	_parentWindow(parentWindow)
{
	_createChaseButton.signal_clicked().
		connect(sigc::mem_fun(*this, &SequenceFrame::onCreateChaseButtonClicked));
	_createChaseButton.set_sensitive(false);
	_sequenceButtonBox.pack_start(_createChaseButton);

	_sequenceInnerBox.pack_start(_sequenceButtonBox, false, false, 5);

	_sequenceList.SetDisplayType(ObjectTree::OnlySequences);
	_sequenceList.SignalSelectionChange().connect(sigc::mem_fun(*this, &SequenceFrame::onSelectedSequenceChanged));

	_sequenceScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_sequenceInnerBox.pack_start(_sequenceList);

	_sequenceOuterBox.pack_start(_sequenceInnerBox);

	_sequenceOuterBox.pack_start(_nameFrame, false, false, 2);

	_sequenceFrame.add(_sequenceOuterBox);

	add1(_sequenceFrame);
	show_all_children();
}


SequenceFrame::~SequenceFrame()
{ }

void SequenceFrame::onCreateChaseButtonClicked()
{
	NamedObject* object = _sequenceList.SelectedObject();
	if(object)
	{
		Sequence* sequence = dynamic_cast<Sequence*>(object);
		if(sequence && sequence->Size() != 0)
		{
			Folder& parent = sequence->Parent();
			std::unique_lock<std::mutex> lock(_management->Mutex());
			Chase& chase = _management->AddChase(*sequence);
			chase.SetName(sequence->Name());
			parent.Add(chase);

			_management->AddPreset(chase);
			lock.unlock();

			_parentWindow.EmitUpdate();
			_parentWindow.MakeChaseTabActive(chase);
		}
	}
}

void SequenceFrame::onSelectedSequenceChanged()
{
	if(_delayUpdates.IsFirst())
	{
		NamedObject* object = _sequenceList.SelectedObject();
		if(object)
		{
			_nameFrame.SetNamedObject(*object);
			
			Sequence* sequence = dynamic_cast<Sequence*>(object);
			if(sequence)
				_createChaseButton.set_sensitive(true);
			else
				_createChaseButton.set_sensitive(false);
		}
		else
		{
			_nameFrame.SetNoNamedObject();
			_createChaseButton.set_sensitive(false);
		}
	}
}

void SequenceFrame::Select(const class Sequence& sequence)
{
	_sequenceList.SelectObject(sequence);
}

