/*****************************************************************************
*
* Copyright (C) 2021 LIPS Corp.
*
* This sample demos how to use OpenNI2 event-based design to capture
* frames to solve infinite time waiting problem. We also demo how to
* recover streaming when device re-connects after USB disconnection
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
#include "ListenerViewer.hpp"

#include <iostream>

using namespace std;
using namespace openni;

class DeviceListener : public DeviceListenerBase
{
    public:
        DeviceListener(std::string uri) : DeviceListenerBase(uri)
        {}

        void onDeviceConnected( const DeviceInfo *info )
        {
            cout << endl << "++++++++++ DeviceConnect ++++++++++" << endl;
            cout << "Vendor : " << info->getVendor() << endl;
            cout << "Name   : " << info->getName() << endl;
            cout << "VID/PID: " << hex << info->getUsbVendorId() << " / " << info->getUsbProductId() << endl;
            cout << "URI    : " << info->getUri() << endl;
            cout << "+++++++++++++++++++++++++++++++++++" << endl << endl;

            const string uri( info->getUri() );
            if ( !device )
            {
                if ( !deviceURI.empty() && deviceURI.compare( uri ) != 0 )
                {
                    cout << "Original device disconnected. New device detected." << endl;
                    cout << "Original Device: " << deviceURI << endl;
                    cout << "New Device: " << uri << endl;
                }

                lock_guard<mutex> guard( mtx );
                deviceURI.assign( uri );
                execution = DeviceStreamExecution::OPEN_AND_START;
            }
        }

        void onDeviceDisconnected( const DeviceInfo *info )
        {
            cout << endl << "---------- DeviceDisconnect ----------" << endl;
            cout << "Vendor : " << info->getVendor() << endl;
            cout << "Name   : " << info->getName() << endl;
            cout << "VID/PID: " << hex << info->getUsbVendorId() << " / " << info->getUsbProductId() << endl;
            cout << "URI    : " << info->getUri() << endl;
            cout << "--------------------------------------" << endl << endl;

            const string uri( info->getUri() );
            if ( !deviceURI.empty() && deviceURI.compare( uri ) == 0 )
            {
                lock_guard<mutex> guard( mtx );
                execution = DeviceStreamExecution::STOP_AND_CLOSE;
            }
        }

        void onDeviceStateChanged( const DeviceInfo *info, DeviceState state )
        {
            const string strState[] = { "DEVICE_STATE_OK", "DEVICE_STATE_ERROR", "DEVICE_STATE_NOT_READY", "DEVICE_STATE_EOF" };

            cout << endl << "========== DeviceStateChange ==========" << endl;
            cout << "Vendor : " << info->getVendor() << endl;
            cout << "Name   : " << info->getName() << endl;
            cout << "VID/PID: " << hex << info->getUsbVendorId() << " / " << info->getUsbProductId() << endl;
            cout << "URI    : " << info->getUri() << endl;
            cout << "State  : " << strState[state] << endl;
            cout << "=======================================" << endl << endl;
        }
};

int main(int argc, char** argv)
{
	Status rc = STATUS_OK;

	std::string deviceURI(""); //ANY_DEVICE
	if (argc > 1)
	{
		deviceURI = std::string(argv[1]);
	}

    // Note: Device-listeners should be added before OpenNI module initializes.
    DeviceListener mDeviceListener(deviceURI);
    OpenNI::addDeviceConnectedListener( &mDeviceListener );
    OpenNI::addDeviceDisconnectedListener( &mDeviceListener );
    OpenNI::addDeviceStateChangedListener( &mDeviceListener );

	if (OpenNI::initialize() != STATUS_OK)
	{
		cout << "OPENNI initialization:" << OpenNI::getExtendedError() << endl;
		return EXIT_FAILURE;
	}

    //Let default streams are Depth + IR
	ListenerViewer eventViewer("LIPS OpenNI2 Event-Based Viewer GL", &mDeviceListener, DISPLAY_MODE_DEPTH_IR);

	rc = eventViewer.init();
	if (rc != STATUS_OK)
	{
		cout << "Failed to init sample program. Exiting" << endl;
		OpenNI::shutdown();
		return EXIT_FAILURE;
	}

	eventViewer.run();

    OpenNI::removeDeviceStateChangedListener( &mDeviceListener );
    OpenNI::removeDeviceDisconnectedListener( &mDeviceListener );
    OpenNI::removeDeviceConnectedListener( &mDeviceListener );

	OpenNI::shutdown();

	eventViewer.exit();
	return EXIT_SUCCESS;
}