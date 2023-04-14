/*****************************************************************************
*
* Copyright (C) 2021 LIPS Corp.
*
* Ni2PowerTest
*
* This sample tool demos how to test power modes on LIPSedge cameras
* viewer GUI changes to OpenGL implementation
*
*****************************************************************************/
#include <OpenNI.h>
#include <iostream>
#include <thread>
#include <string>

#include "Viewer.h"
#include "LIPSNICustomProperty.h"

using namespace std;
using namespace openni;

Device device;
VideoStream depth, color, ir;

void input_handler()
{
    string args;
    Status status;

    while ( 1 ) {

        cout << "Power Test: " << endl;
        cout << "  l/L Low-Power Mode" << endl;
        cout << "  n/N Normal Mode" << endl;
        //cout << "  q/Q (to exit program)" << endl;
        cout << "Please input your option: ";
        cin >> args;
        //cout << "Your input command:[" << args << "]" << endl;

        switch(args.at(0))
        {
            case 'l': //Low-Power Mode
            case 'L':
            {
                int cmd = 1; //1=enable
                int cmdSize = sizeof(cmd);
                status = depth.setProperty(LIPS_DEPTH_SENSOR_LOW_POWER_EN, (void *)&cmd, cmdSize );
                if( status == STATUS_OK )
                {
                    cout << endl << "OK! ";
                    cout << "Set device to Low-Power Mode" << endl;
                }
                else
                {
                    cout << endl << "FAIL! (or access denied)" << endl;
                }
                break;
            }
            case 'n': //Normal Mode
            case 'N':
            {
                int cmd = 0;
                int cmdSize = sizeof(cmd);
                status = depth.setProperty(LIPS_DEPTH_SENSOR_LOW_POWER_EN, (void *)&cmd, sizeof(cmd) );
                if( status == STATUS_OK )
                {
                    cout << "OK! ";
                    cout << "Set device to Normal Mode" << endl;
                }
                else
                {
                    cout << "FAIL! (or access denied)" << endl;
                }
                break;
            }
            //case 'q': //quit program can be done by press q/Q on OpenGL window
            //case 'Q':
            //    bInputLoop = false;
            //    break;
            default:
                cout << "Unknown input: " << args.at(0) << " ...Try again" << endl;
                break;
        }

        cout << "*****" << endl;
    }
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

	SampleViewer sampleViewer("LIPSedge 3D Camera Power Test DEMO GL", &device, DISPLAY_MODE_DEPTH_IR, &depth, &color, &ir);

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

	depth.destroy();
	//color.destroy();
	ir.destroy();
	device.close();
	OpenNI::shutdown();

	sampleViewer.exit();
	input_loop.detach();

	return EXIT_SUCCESS;
}