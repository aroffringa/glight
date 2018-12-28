#include "sequenceframe.h"

#include "../libtheatre/sequence.h"
#include "../libtheatre/chase.h"

#include "programwindow.h"

SequenceFrame::SequenceFrame(Management &management, ProgramWindow &parentWindow)
	:
	_sequenceFrame("All sequences"),
	_createChaseButton("Create chase"),
	_nameFrame(management),
	_management(management), _parentWindow(parentWindow)
{
	_createChaseButton.signal_clicked().
		connect(sigc::mem_fun(*this, &SequenceFrame::onCreateChaseButtonClicked));
	_createChaseButton.set_sensitive(false);
	_sequenceButtonBox.pack_start(_createChaseButton);
	_createChaseButton.show();

	_sequenceInnerBox.pack_start(_sequenceButtonBox, false, false, 5);
	_sequenceButtonBox.show();

	_sequenceListModel =
    Gtk::ListStore::create(_sequenceListColumns);

	_sequenceListView.set_model(_sequenceListModel);
	_sequenceListView.append_column("Sequence", _sequenceListColumns._title);
	_sequenceListView.get_selection()->signal_changed().connect(sigc::mem_fun(*this, &SequenceFrame::onSelectedSequenceChanged));
	_sequenceScrolledWindow.add(_sequenceListView);
	_sequenceListView.show();

	_sequenceScrolledWindow.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_sequenceInnerBox.pack_start(_sequenceScrolledWindow);
	_sequenceScrolledWindow.show();

	_sequenceOuterBox.pack_start(_sequenceInnerBox);
	_sequenceInnerBox.show();

	_nameFrame.SignalNameChange().connect(sigc::mem_fun(*this, &SequenceFrame::onNameChange));
	_sequenceOuterBox.pack_start(_nameFrame, false, false, 2);
	_nameFrame.show();

	_sequenceFrame.add(_sequenceOuterBox);
	_sequenceOuterBox.show();

	add1(_sequenceFrame);
	_sequenceFrame.show();
}


SequenceFrame::~SequenceFrame()
{
}

void SequenceFrame::fillSequenceList()
{
	_sequenceListModel->clear();

	std::lock_guard<std::mutex> lock(_management.Mutex());
	const std::vector<std::unique_ptr<Sequence>>&
		sequences = _management.Sequences();
	for(const std::unique_ptr<Sequence>& sequence : sequences)
	{
		Gtk::TreeModel::iterator iter = _sequenceListModel->append();
		Gtk::TreeModel::Row row = *iter;
		row[_sequenceListColumns._title] = sequence->Name();
		row[_sequenceListColumns._sequence] = sequence.get();
	}
}

void SequenceFrame::onCreateChaseButtonClicked()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
    _sequenceListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
	{
		Sequence *sequence = (*selected)[_sequenceListColumns._sequence];

		std::unique_lock<std::mutex> lock(_management.Mutex());
		Chase &chase = _management.AddChase(*sequence);
		chase.SetName(sequence->Name());

		_management.AddPreset(chase);
		lock.unlock();

		_parentWindow.UpdateChaseList();
		_parentWindow.MakeChasesTabActive();
	}
}

void SequenceFrame::onSelectedSequenceChanged()
{
	Glib::RefPtr<Gtk::TreeSelection> selection =
    _sequenceListView.get_selection();
	Gtk::TreeModel::iterator selected = selection->get_selected();
	if(selected)
	{
		Sequence *sequence = (*selected)[_sequenceListColumns._sequence];
		_nameFrame.SetNamedObject(*sequence);

		_createChaseButton.set_sensitive(true);
	}
	else
	{
		_nameFrame.SetNoNamedObject();

		_createChaseButton.set_sensitive(false);
	}
}


