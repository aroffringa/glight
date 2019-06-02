#include "visualizationwindow.h"

#include "../libtheatre/management.h"
#include "../libtheatre/dmxdevice.h"
#include "../libtheatre/fixture.h"
#include "../libtheatre/theatre.h"
#include "../libtheatre/valuesnapshot.h"

#include <glibmm/main.h>

VisualizationWindow::VisualizationWindow(Management* management) :
	_management(management),
	_dryManagement(nullptr),
	_isInitialized(false), _isTimerRunning(false)
{
	set_title("Glight - visualization");
	set_default_size(600, 200);

	_drawingArea.signal_draw().connect(sigc::mem_fun(*this, &VisualizationWindow::onExpose));

	add(_drawingArea);
	_drawingArea.show();
}

VisualizationWindow::~VisualizationWindow()
{
	_timeoutConnection.disconnect();
}

void VisualizationWindow::initialize()
{
	queue_draw();
	_isInitialized = true;

	if(!_isTimerRunning)
	{
		_timeoutConnection = Glib::signal_timeout().connect( sigc::mem_fun(*this, &VisualizationWindow::onTimeout), 40);
		_isTimerRunning = true;
	}
}

bool VisualizationWindow::onExpose(const Cairo::RefPtr<Cairo::Context>& context)
{
	if(!_isInitialized)
		initialize();

	drawAll(context);
	return true;
}

void VisualizationWindow::drawManagement(const Cairo::RefPtr< Cairo::Context>& cairo, Management& management, size_t yOffset, size_t height)
{
	double width = _drawingArea.get_width();

	const Theatre &theatre = management.Theatre();
	const ValueSnapshot snapshot = management.Snapshot();
	const std::vector<std::unique_ptr<Fixture>>& fixtures = theatre.Fixtures();
	if(!fixtures.empty())
	{
		Position extend = theatre.Extend();
		double radius = std::min(width/(extend.X()*2.0), height/(extend.Y()*2.0));
		for(const std::unique_ptr<Fixture>& f : fixtures)
		{
			Color c = f->GetColor(snapshot);

			cairo->set_source_rgb((double) c.Red()/224.0+0.125, (double) c.Green()/224.0+0.125, (double) c.Blue()/224.0+0.125);

			double x = ((double) f->Position().X() + 0.5) * (width / extend.X());
			double y = ((double) f->Position().Y() + 0.5) * (height / extend.Y());
			cairo->arc(x, y + yOffset, radius*0.8, 0.0, 2.0 * M_PI);
			cairo->fill();
		}
	}
}

void VisualizationWindow::drawAll(const Cairo::RefPtr< Cairo::Context>& cairo)
{
	double
		width = _drawingArea.get_width(),
		height = _drawingArea.get_height();

	cairo->set_source_rgba(0,0,0,1);
	cairo->rectangle(0, 0, width, height);
	cairo->fill();
	
	if(_dryManagement==nullptr)
	{
		drawManagement(cairo, *_management, 0, height);
	}
	else {
		drawManagement(cairo, *_management, 0, height/2.0);
		drawManagement(cairo, *_dryManagement, height/2.0, height/2.0);
	}
}

