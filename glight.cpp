#include <iostream>
#include <cstdlib>

#include "libtheatre/dmxdevice.h"

#include "gui/application.h"
#include "writer.h"

using namespace std;

int main(int argc, char *argv[])
{
	Writer::CheckXmlVersion();

	Application application;
	application.Run(argc, argv);

  return EXIT_SUCCESS;
}
