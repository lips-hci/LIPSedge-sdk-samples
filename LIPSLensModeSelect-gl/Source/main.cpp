/*****************************************************************************
*
* Copyright (C) 2021 LIPS Corp.
*
* This sample demos how to select and switch between lens mode on-the-fly
*
* Instead of modifying parameter lens_mode in ModuleConfig.json
* You can use programming way (OpenNI VideoStream get/set Property)
* to control Depth camera to switch to new lens mode
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
* Console input:
*      0/1/2/3: to select lens mode you want
*
*****************************************************************************/
#include <OpenNI.h>
#include "Viewer.h"
#include "LIPSNICustomProperty.h"

#include <iostream>
#include <limits>

using namespace std;
using namespace openni;

Device device;
VideoStream depth, color, ir;

void input_handler()
{
	//User interactive input to select next lens mode
	Status rc;

	while(1)
	{
		int next_mode, cur_mode;
		int data_size = sizeof(cur_mode);
		rc = device.getProperty(LIPS_DEVICE_CONFIG_LENS_MODE, &cur_mode, &data_size);
		if (rc == STATUS_NO_DEVICE)
		{
			//maybe device is BUSY on switching...give it some time
			std::this_thread::sleep_for(std::chrono::seconds(3));
			continue;
		}

		cout << endl << "====================================";
		cout << endl << "Current lens mode: " << cur_mode;
		cout << endl << "Please input next lens mode (0/1/2/3): ";

		int din = 99;
		char c;
    	if ( scanf("%d", &din) != EOF ) {
			while ( (c = getchar()) != '\n' && c != EOF ) {;}
		}
		//cout << endl << "[DEBUG] input " << din << endl;

		if (din == 0 || din == 1 || din == 2 || din == 3)
		{
			next_mode = din;
			cout << endl << "Your input lens mode is " << next_mode;
			cout << endl << "Waiting for operation to be done..." << endl << endl;
			rc = device.setProperty<int>(LIPS_DEVICE_CONFIG_LENS_MODE, next_mode);
			if (rc == STATUS_OK)
			{
				cout << endl << "Change to lens mode " << next_mode << " successfully." << endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
		}
		else
		{
			//printf("[DEBUG] you input %d, %c, %d\n", din, din, (int)din - 48);
			cout << endl << "Input is not decimal number or invalid. Try again." << endl;
		}
		cout << endl << "====================================" << endl;
	};
}

int main(int argc, char** argv)
{
	Status rc = STATUS_OK;

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
	//cout << endl << "[DEBUG] OPENNI initialized" << endl;

	//cout << endl << "[DEBUG] openning device" << endl;
	rc = device.open(deviceURI);
	if (rc != STATUS_OK)
	{
		cout << "Device open failed:" << OpenNI::getExtendedError() << endl;
		OpenNI::shutdown();
		return EXIT_FAILURE;
	}
	//cout << endl << "[DEBUG] ......done" << endl;

#if 0
	//Demo how to select lens mode when Depth stream begins
	cout << endl << "[TEST] setting lens mode to 2" << endl;
	int begin_mode = 2; //change to any lens mode you want, e.g. 0,1,2,or 3
	rc = device.setProperty<int>(LIPS_DEVICE_CONFIG_LENS_MODE, begin_mode);
	cout << endl << "[TEST] ......done" << endl;
#endif

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

	SampleViewer sampleViewer("LIPSedge 3D Camera Lens Mode Select DEMO GL", &device, DISPLAY_MODE_DEPTH_IR, &depth, &color, &ir);

	rc = sampleViewer.init();
	if (rc != STATUS_OK)
	{
		cout << "Failed to init sample program. Exiting" << endl;
		OpenNI::shutdown();
		return EXIT_FAILURE;
	}

	//Run input handler to wait for user input
	thread input_loop = thread(input_handler);

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
	input_loop.detach();

	return EXIT_SUCCESS;
}