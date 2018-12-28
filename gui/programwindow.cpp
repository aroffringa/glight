#include "programwindow.h"

#include "../libtheatre/management.h"

#include "showwindow.h"
#include "presetsframe.h"
#include "sequenceframe.h"
#include "chaseframe.h"

ProgramWindow::ProgramWindow(Management &management, ShowWindow &showWindow)
	: _showWindow(showWindow)
{
	set_title("Glight - programming");

	set_default_size(400,400);

	_presetsFrame = new PresetsFrame(management, *this);
	_notebook.append_page(*_presetsFrame, "Presets");
	_presetsFrame->show();

	_sequenceFrame = new SequenceFrame(management, *this);
	_notebook.append_page(*_sequenceFrame, "Sequences");
	_sequenceFrame->show();

	_chaseFrame = new ChaseFrame(management, *this);
	_notebook.append_page(*_chaseFrame, "Chases");
	_chaseFrame->show();

	add(_notebook);
	_notebook.show();
}

ProgramWindow::~ProgramWindow()
{
	delete _chaseFrame;
	delete _sequenceFrame;
	delete _presetsFrame;
}

void ProgramWindow::UpdateSequenceList()
{
	_sequenceFrame->Update();
}

void ProgramWindow::UpdateChaseList()
{
	_chaseFrame->Update();
}

void ProgramWindow::ForwardUpdateAfterPresetRemoval()
{
	_showWindow.EmitUpdateAfterPresetRemoval();
}

void ProgramWindow::ForwardUpdateAfterAddPreset()
{
	_showWindow.EmitUpdateAfterAddPreset();
}

void ProgramWindow::Update()
{
	_presetsFrame->Update();
	_sequenceFrame->Update();
	_chaseFrame->Update();
}

void ProgramWindow::UpdateAfterPresetRemoval()
{
	_presetsFrame->UpdateAfterPresetRemoval();
	_sequenceFrame->UpdateAfterPresetRemoval();
	_chaseFrame->UpdateAfterPresetRemoval();
}
