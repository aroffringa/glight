#include "application.h"

#include <iostream>

#include <gtkmm/main.h>

#include "showwindow.h"

//#include "../libtheatre/ftdidevice.h"
#include "../theatre/oladevice.h"
#include "../theatre/dummydevice.h"

Application::Application()
{ }

void Application::Run(int argc, char *argv[])
{
	Gtk::Main kit(argc, argv);
	//std::unique_ptr<DmxDevice> device(new FtdiDevice());
	std::unique_ptr<DmxDevice> device;
	bool isOpen = false;
	try {
		device.reset(new OLADevice());
		device->Open();
		isOpen = device->IsOpen();
	}
	catch(std::exception& e)
	{
		std::cerr << "DMX device threw exception: " << e.what() << '\n';
	}
	if(!isOpen)
	{
		std::cerr << "DMX device not working, switching to dummy device.\n";
		device.reset(new DummyDevice());
	}
	ShowWindow window(std::move(device));
	if(argc > 1)
	{
		window.OpenFile(argv[1]);
	}
	kit.run(window);
}
