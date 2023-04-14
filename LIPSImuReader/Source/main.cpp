/*****************************************************************************
*
* Copyright (C) 2022 LIPS Corp.
*
* LIPSImuReader
*
* This sample demonstrates how to read Inertial measurement unit (IMU: accelerometer and gyroscope)
* sensor data by OpenNI2 API
*
*****************************************************************************/

#include <OpenNI.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <thread>

#include "LIPSNICustomProperty.h"

using namespace std;
using namespace openni;

int main( int argc, char* argv[] )
{
    int total_loops = 100;
    int sleep_ms = 100;

    if ( argc == 3 )
    {
        total_loops = atoi( argv[1] );
        sleep_ms = atoi( argv[2] );
    }
    else
    {
        cout << endl << endl;
        cout << "another usage: LIPSImuReader [total loops] [sleep ms]" << endl;
        cout << "e.g. LIPSImuReader 100 100" << endl;
        cout << "read IMU 100 times and take 100ms sleep between each read" << endl;
        cout << endl << endl;
    }

    if ( STATUS_OK != OpenNI::initialize() )
    {
        cout << "After initialization: " << OpenNI::getExtendedError() << endl;
        return 1;
    }

    Device device;
    if ( STATUS_OK != device.open( ANY_DEVICE ) )
    {
        cout << "Cannot open device: " << OpenNI::getExtendedError() << endl;
        return 1;
    }

    if ( device.isPropertySupported( LIPS_DEVICE_PROPERTY_IMUDATA ) )
    {
        int loops = 0;
        while ( ++loops <= total_loops )
        {
            float IMUdata[6] = { 0 };
            int size = sizeof( IMUdata );

            if ( device.isValid() )
            {
                cout << "[" << loops << "] ";
                if ( STATUS_OK == device.getProperty( LIPS_DEVICE_PROPERTY_IMUDATA, IMUdata, &size ) )
                {
                    cout << "Gyro (X,Y,Z) = (";
                    cout << IMUdata[0] << ", " << IMUdata[1] << ", " << IMUdata[2] << "),\t";
                    cout << "Accel (X,Y,Z) = (";
                    cout << IMUdata[3] << ", " << IMUdata[4] << ", " << IMUdata[5] << ")" << endl;
                }
                else
                {
                    cout << "Fail to read IMU data or unsupported!" << endl;
                }
            }

            std::this_thread::sleep_for( std::chrono::milliseconds( sleep_ms ) );
        }
        device.close();
        OpenNI::shutdown();
        return 0;
    }
    else
    {
        cout << "Device is NOT support for reading IMU data!" << endl;
    }

    VideoStream depth;
    if ( STATUS_OK != depth.create( device, SENSOR_DEPTH ) )
    {
        cout << "Cannot create depth stream on device: " << OpenNI::getExtendedError() << endl;
        return 1;
    }

    depth.start();

    int loops = 0;
    while ( ++loops <= total_loops )
    {
        float IMUdata[6] = { 0 };
        int size = sizeof( IMUdata );

        if ( depth.isValid() )
        {
            cout << "[" << loops << "] ";
            if ( STATUS_OK == depth.getProperty( LIPS_STREAM_PROPERTY_IMUDATA, IMUdata, &size ) )
            {
                cout << "Gyro (X,Y,Z) = (";
                cout << IMUdata[0] << ", " << IMUdata[1] << ", " << IMUdata[2] << "),\t";
                cout << "Accel (X,Y,Z) = (";
                cout << IMUdata[3] << ", " << IMUdata[4] << ", " << IMUdata[5] << ")" << endl;
            }
            else
            {
                cout << "Fail to read IMU data or unsupported!" << endl;
            }
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( sleep_ms ) );
    }

    depth.destroy();
    device.close();
    OpenNI::shutdown();

    return 0;
}
