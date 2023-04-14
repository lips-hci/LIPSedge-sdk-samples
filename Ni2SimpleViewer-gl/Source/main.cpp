/*****************************************************************************
*
* Copyright (C) 2021 LIPS Corp.
*
* This simple viewer example draws side-by-sdie frames by OpenGL
* and extends hot keys to swith different cameras by number keys
*
* Viewer hot keys:
*      1: Depth and Image (default)
*      2: Depth
*      3: Image
*      4: Image with Depth overlay
*      5: Depth and IR
*    H/h: show/close help menu
*    M/m: mirror on/off
*  Esc/q: exit program
*
*****************************************************************************/
#include <OpenNI.h>
#include "Viewer.h"

#include <iostream>

using namespace std;
using namespace openni;

int main(int argc, char** argv)
{
	Status rc = STATUS_OK;

	Device device;
	VideoStream depth, color, ir;
	const char* deviceURI = ANY_DEVICE;
	if (argc > 1)
	{
		deviceURI = argv[1];
	}

	if (OpenNI::initialize() != STATUS_OK)
	{
		cout << "OPENNI initialization:" << OpenNI::getExtendedError() << endl;
		return EXIT_FAILURE;
	}

	rc = device.open(deviceURI);
	if (rc != STATUS_OK)
	{
		cout << "Device open failed:" << OpenNI::getExtendedError() << endl;
		OpenNI::shutdown();
		return EXIT_FAILURE;
	}

	//TIPS: start Color stream prior to Depth stream can safe camera launch time
	rc = color.create(device, SENSOR_COLOR);
	if (rc == STATUS_OK)
	{
		rc = color.start();
		if (rc != STATUS_OK)
		{
			cout << "Couldn't start color stream:" << OpenNI::getExtendedError() << endl;
			color.destroy();
		}
	}
	else
	{
		cout << "Couldn't find color stream:" << OpenNI::getExtendedError() << endl;
	}

	rc = depth.create(device, SENSOR_DEPTH);
	if (rc == STATUS_OK)
	{
		rc = depth.start();
		if (rc != STATUS_OK)
		{
			cout << "Couldn't start depth stream:" << OpenNI::getExtendedError() << endl;
			depth.destroy();
		}
	}
	else
	{
		cout << "Couldn't find depth stream:" << OpenNI::getExtendedError() << endl;
	}

	rc = ir.create(device, SENSOR_IR);
	if (rc == STATUS_OK)
	{
		rc = ir.start();
		if (rc != STATUS_OK)
		{
			cout << "Couldn't start IR stream:" << OpenNI::getExtendedError() << endl;
			ir.destroy();
		}
	}
	else
	{
		cout << "Couldn't find IR stream:" << OpenNI::getExtendedError() << endl;
	}

	if (!depth.isValid())
	{
		cout << "No valid Depth stream. Exiting" << endl;
		OpenNI::shutdown();
		return EXIT_FAILURE;
	}

	SampleViewer sampleViewer("LIPS OpenNI2 Simple Viewer GL", &device, DISPLAY_MODE_DEPTH_IMAGE, &depth, &color, &ir);

	rc = sampleViewer.init();
	if (rc != STATUS_OK)
	{
		cout << "Failed to init sample program. Exiting" << endl;
		OpenNI::shutdown();
		return EXIT_FAILURE;
	}

	sampleViewer.run();

	depth.stop();
	color.stop();
	ir.stop();

	depth.destroy();
	color.destroy();
	ir.destroy();

	device.close();
	OpenNI::shutdown();

	sampleViewer.exit();
	return EXIT_SUCCESS;
}