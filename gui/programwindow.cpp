#include "programwindow.h"

#include "../libtheatre/management.h"

#include "showwindow.h"
#include "presetsframe.h"
#include "sequenceframe.h"
#include "chaseframe.h"

ProgramWindow::ProgramWindow(Management& management, ShowWindow& showWindow) :
	_presetsFrame(new PresetsFrame(management, *this)),
	_sequenceFrame(new SequenceFrame(management, *this)),
	_chaseFrame(new ChaseFrame(management, *this)),
	_showWindow(showWindow)
{
	set_title("Glight - programming");

	set_default_size(400,400);

	_notebook.append_page(*_presetsFrame, "Presets");
	_presetsFrame->show();

	_notebook.append_page(*_sequenceFrame, "Sequences");
	_sequenceFrame->show();

	_notebook.append_page(*_chaseFrame, "Chases");
	_chaseFrame->show();

	add(_notebook);
	_notebook.show();
}

ProgramWindow::~ProgramWindow()
{ }

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

void ProgramWindow::ChangeManagement(class Management& management)
{
	_presetsFrame->ChangeManagement(management);
	_sequenceFrame->ChangeManagement(management);
	_chaseFrame->ChangeManagement(management);
}
