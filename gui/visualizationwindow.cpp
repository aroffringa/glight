#include "visualizationwindow.h"

#include "../libtheatre/management.h"
#include "../libtheatre/dmxdevice.h"
#include "../libtheatre/fixture.h"
#include "../libtheatre/theatre.h"
#include "../libtheatre/valuesnapshot.h"

#include <glibmm/main.h>

VisualizationWindow::VisualizationWindow(Management &management) : _management(management), _isInitialized(false), _isTimerRunning(false)
{
	set_title("Glight - visualization");

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
	//Gdk::Pixmap::create(_drawingArea.get_window(), _drawingArea.get_width(), _drawingArea.get_height());
	//_buffer = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, _drawingArea.get_width(), _drawingArea.get_height());
	queue_draw();
	_isInitialized = true;
	//_bufferWidth = _drawingArea.get_width();
	//_bufferHeight = _drawingArea.get_height();

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

	draw(context);
	return true;
}

void VisualizationWindow::draw(const Cairo::RefPtr< Cairo::Context>& cairo)
{
	double
		width = _drawingArea.get_width(),
		height = _drawingArea.get_height();

	cairo->set_source_rgba(0,0,0,1);
	cairo->rectangle(0, 0, width, height);
	cairo->fill();

	Theatre &theatre = _management.Theatre();
	ValueSnapshot snapshot = _management.Snapshot();
	const std::vector<std::unique_ptr<Fixture>>& fixtures = theatre.Fixtures();
	int position = 0;
	size_t fixtureCount = fixtures.size();
	double minDistance = std::min(width/(fixtureCount*2.0), height/2.0);
	for(const std::unique_ptr<Fixture>& f : fixtures)
	{
		Color c = f->GetColor(snapshot);

		cairo->set_source_rgb((double) c.Red()/224.0+0.125, (double) c.Green()/224.0+0.125, (double) c.Blue()/224.0+0.125);

		double x = ((double) position + 0.5) * (width / fixtureCount);
		cairo->arc(x, height/2.0, minDistance*0.8, 0.0, 2.0 * M_PI);
		cairo->fill();

		++position;
	}
}
