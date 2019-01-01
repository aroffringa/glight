#ifndef VISUALIZATIONWINDOW_H
#define VISUALIZATIONWINDOW_H

#include <gdkmm/pixbuf.h>

#include <gtkmm/drawingarea.h>
#include <gtkmm/window.h>

/**
	@author Andre Offringa
*/
class VisualizationWindow : public Gtk::Window {
	public:
		VisualizationWindow(class Management* management);
		~VisualizationWindow();

		void Update()
		{
			queue_draw();
		}
		
		void SetDryMode(class Management* dryManagement)
		{
			_dryManagement = dryManagement;
			Update();
		}
		void SetRealMode()
		{
			_dryManagement = nullptr;
			Update();
		}
	private:
		Gtk::DrawingArea _drawingArea;

		Management* _management;
		Management* _dryManagement;
		bool _isInitialized, _isTimerRunning;
		sigc::connection _timeoutConnection;

		void initialize();
		void draw(const Cairo::RefPtr< Cairo::Context>& cairo);
		bool onExpose(const Cairo::RefPtr<Cairo::Context>& context);
		bool onTimeout()
		{
			Update();
			return true;
		}
};

#endif
