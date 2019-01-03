#ifndef PROGRAMWINDOW_H
#define PROGRAMWINDOW_H

#include <gtkmm/notebook.h>
#include <gtkmm/window.h>

#include <memory>

/**
	@author Andre Offringa
*/
class ProgramWindow : public Gtk::Window {
	public:
		ProgramWindow(class Management& management, class ShowWindow& showWindow);
		~ProgramWindow();

		void MakeSequenceTabActive() { _notebook.set_current_page(1); }
		void MakeChasesTabActive() { _notebook.set_current_page(2); }
		void UpdateSequenceList();
		void UpdateChaseList();
		void ForwardUpdateAfterPresetRemoval();
		void ForwardUpdateAfterAddPreset();
		void Update();
		void UpdateAfterPresetRemoval();
		
		void ChangeManagement(class Management &management);
	private:

		Gtk::Notebook _notebook;
		std::unique_ptr<class PresetsFrame> _presetsFrame;
		std::unique_ptr<class SequenceFrame> _sequenceFrame;
		std::unique_ptr<class ChaseFrame> _chaseFrame;

		class ShowWindow& _showWindow;
};

#endif
