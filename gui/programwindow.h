#ifndef PROGRAMWINDOW_H
#define PROGRAMWINDOW_H

#include <gtkmm/notebook.h>
#include <gtkmm/window.h>

/**
	@author Andre Offringa
*/
class ProgramWindow : public Gtk::Window {
	public:
		ProgramWindow(class Management &management, class ShowWindow &showWindow);
		~ProgramWindow();

		void MakeSequenceTabActive() { _notebook.set_current_page(1); }
		void MakeChasesTabActive() { _notebook.set_current_page(2); }
		void UpdateSequenceList();
		void UpdateChaseList();
		void ForwardUpdateAfterPresetRemoval();
		void ForwardUpdateAfterAddPreset();
		void Update();
		void UpdateAfterPresetRemoval();
	private:

		Gtk::Notebook _notebook;
		class PresetsFrame *_presetsFrame;
		class SequenceFrame *_sequenceFrame;
		class ChaseFrame *_chaseFrame;

		class ShowWindow &_showWindow;
};

#endif
