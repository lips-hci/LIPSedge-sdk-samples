/*****************************************************************************
*
* Copyright (C) 2021 LIPS Corp.
*
* Ni2CameraMatrix
*
* This example demonstrates how to get camera intrinsic and extrinsic by OpenNI2 API.
*
*****************************************************************************/
#include <stdio.h>
#include <OpenNI.h>

#include "LIPSNICustomProperty.h"

#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace openni;

#define MAX_STR_SIZE 4096

#ifdef WIN32
#define snprintf sprintf_s
#endif

string formatIntrinsic(const string strSensorName, VideoStream &vs, VideoMode &vm )
{
    int resX, resY;
    double fx, fy, cx, cy;
    float hfov, vfov;
    RadialDistortionCoeffs radialDistCoeffs;
    TangentialDistortionCoeffs tangentialDistCoeffs;

    char outBuf[MAX_STR_SIZE] = { 0 };
    int pos = 0;

    resX = vm.getResolutionX();
    resY = vm.getResolutionY();

    string strPfmt = "";
    switch ( vm.getPixelFormat() )
    {
        // Depth
        case PIXEL_FORMAT_DEPTH_100_UM:
            strPfmt = "Depth-16bit,0.1mm";
            break;
        case PIXEL_FORMAT_DEPTH_1_MM:
            strPfmt = "Depth-16bit,1mm";
            break;

	    // Color
	    case PIXEL_FORMAT_RGB888:
            strPfmt = "RGB888";
            break;
	    case PIXEL_FORMAT_YUV422:
            strPfmt = "YUV422";
            break;
	    case PIXEL_FORMAT_JPEG:
            strPfmt = "JPEG";
            break;
	    case PIXEL_FORMAT_YUYV:
            strPfmt = "YUYV";
            break;

        // Infrared
	    case PIXEL_FORMAT_GRAY8:
            strPfmt = "Grayscale-8bit";
            break;
	    case PIXEL_FORMAT_GRAY16:
            strPfmt = "Grayscale-16bit";
            break;
        default:
            strPfmt = "Unknown";
            break;
    }

    vs.getProperty(ONI_STREAM_PROPERTY_HORIZONTAL_FOV, &hfov);
    vs.getProperty(ONI_STREAM_PROPERTY_VERTICAL_FOV, &vfov);

    vs.getProperty(LIPS_STREAM_PROPERTY_FOCAL_LENGTH_X, &fx);
    vs.getProperty(LIPS_STREAM_PROPERTY_FOCAL_LENGTH_Y, &fy);

    vs.getProperty(LIPS_STREAM_PROPERTY_PRINCIPAL_POINT_X, &cx);
    vs.getProperty(LIPS_STREAM_PROPERTY_PRINCIPAL_POINT_Y, &cy);

    vs.getProperty(LIPS_STREAM_PROPERTY_RADIAL_DISTORTION, &radialDistCoeffs);
    vs.getProperty(LIPS_STREAM_PROPERTY_TANGENTIAL_DISTORTION, &tangentialDistCoeffs);

    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "\n Intrinsic of \"%s\" / %dx%d / %s\n",
                        strSensorName.c_str(), resX, resY, strPfmt.c_str());
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  width:\t%d\n", resX);
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  height:\t%d\n", resY);
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  Fx:\t\t%f\n", fx);
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  Fy:\t\t%f\n", fy);
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  Cx:\t\t%f\n", cx);
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  Cy:\t\t%f\n", cy);
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  HFOV:\t\t%.2f\n", radian2deg(hfov));
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  VFOV:\t\t%.2f\n", radian2deg(vfov));
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, " Distortion Coeffs:\n");
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  Radial:\tk1\tk2\tk3\tk4\tk5\n");
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "    %f\t%f\t%f\t%f\t%f\n",
                        radialDistCoeffs.k1, radialDistCoeffs.k2, radialDistCoeffs.k3, radialDistCoeffs.k4, radialDistCoeffs.k5 );
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  Tangential:\tp1\tp2\n");
    pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "    %f\t%f\n", tangentialDistCoeffs.p1, tangentialDistCoeffs.p2);

    return string(outBuf);
}

string formatExtrinsic(SensorType from, VideoStream &vs, Array<SensorType> &others )
{
    CameraExtrinsicMatrix extrinsic;
    string strSensorTo = "Unknown";
    string strSensorFrom = "Unknown";
    char outBuf[MAX_STR_SIZE] = { 0 };
    int pos = 0;

    if (from == SENSOR_DEPTH)
        strSensorFrom = "Depth";
    else if (from == SENSOR_IR)
        strSensorFrom = "IR";
    else if (from == SENSOR_COLOR)
        strSensorFrom = "Color";

    for (int i = 0; i < others.getSize(); i++)
    {
        if (others[i] == from)
        {
            //skip same type sensor
            continue;
        }
        else if (others[i] == SENSOR_IR)
        {
            strSensorTo = "IR";
            vs.getProperty(LIPS_STREAM_PROPERTY_EXTRINSIC_TO_IR, &extrinsic);
        }
        else if (others[i] == SENSOR_DEPTH)
        {
            strSensorTo = "Depth";
            vs.getProperty(LIPS_STREAM_PROPERTY_EXTRINSIC_TO_DEPTH, &extrinsic);
        }
        else if (others[i] == SENSOR_COLOR)
        {
            strSensorTo = "Color";
            vs.getProperty(LIPS_STREAM_PROPERTY_EXTRINSIC_TO_COLOR, &extrinsic);
        }

        pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "\n Extrinsic from \"%s\" To \"%s\" :\n",
                        strSensorFrom.c_str(), strSensorTo.c_str() );
        pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  Rotation Matrix:\n");
        pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "   %8.6f\t%8.6f\t%8.6f\n",
                        extrinsic.rotation[0][0], extrinsic.rotation[0][1], extrinsic.rotation[0][2]);

        pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "   %8.6f\t%8.6f\t%8.6f\n",
                        extrinsic.rotation[1][0], extrinsic.rotation[1][1], extrinsic.rotation[1][2]);

        pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "   %8.6f\t%8.6f\t%8.6f\n",
                        extrinsic.rotation[2][0], extrinsic.rotation[2][1], extrinsic.rotation[2][2]);

        pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "  Translation Vector:\n");

        pos += snprintf(outBuf + pos, sizeof(outBuf) - pos, "   %8.6f\t%8.6f\t%8.6f\n",
                        extrinsic.translation[0], extrinsic.translation[1], extrinsic.translation[2]);
    }

    return string(outBuf);
}

static inline vector<VideoMode> trimVideoModes( const Array<VideoMode>& allvmodes )
{
    vector<VideoMode> vmodes;
    for (int i = 0; i < allvmodes.getSize(); i++ )
    {
        bool duplicated = false;
        for (auto const& v : vmodes) {
            if ( v.getResolutionX() == allvmodes[i].getResolutionX()
                && v.getResolutionY() == allvmodes[i].getResolutionY() )
            {
                duplicated = true;
                break;
            }
        }
        if (!duplicated)
            vmodes.push_back( allvmodes[i] );
    }
    return vmodes;
}

int main(int argc, char* argv[])
{
    if (STATUS_OK != OpenNI::initialize())
    {
        cout << "After initialization: " << OpenNI::getExtendedError() << endl;
        return 1;
    }

    Device device;
    if (STATUS_OK != device.open(ANY_DEVICE))
    {
        cout << "Cannot open device: " << OpenNI::getExtendedError() << endl;
        return 1;
    }

    vector<SensorType> supportedSensors;
    if ( device.hasSensor( SENSOR_DEPTH ) )
        supportedSensors.push_back(SENSOR_DEPTH);
    if ( device.hasSensor( SENSOR_IR ) )
        supportedSensors.push_back(SENSOR_IR);
    if ( device.hasSensor( SENSOR_COLOR ) )
        supportedSensors.push_back(SENSOR_COLOR);
    Array<SensorType> otherSensors( &supportedSensors[0], supportedSensors.size() );

    cout << endl;
    cout << "=== Camera matrix ===" << endl;

    if ( device.hasSensor( SENSOR_DEPTH ) )
    {
        VideoStream vsDepth;
        if (STATUS_OK != vsDepth.create(device, SENSOR_DEPTH))
        {
            cout << "Cannot create depth stream on device: " << OpenNI::getExtendedError() << endl;
            //return 1;
        }
        else
        {
            cout << "Intrinsic Parameters:" << endl;

            const SensorInfo *sInfo = device.getSensorInfo( SENSOR_DEPTH );
            const Array<VideoMode>& allvmodes = sInfo->getSupportedVideoModes();

            //only pick different video sizes (w x h)
            vector<VideoMode> vmodes = trimVideoModes( allvmodes );
#if 0
            for (int i = 0; i < allvmodes.getSize(); i++ )
            {
                for (auto const& i : vmodes) {
                    if ( i.getResolutionX() == allvmodes[i].getResolutionX()
                        && i.getResolutionY() == allvmodes[i].getResolutionY() )
                        break;
                }
                vmodes.push_back( allvmodes[i] );
            }
#endif
            for ( auto it = vmodes.begin(); it != vmodes.end(); it++ )
            {
                vsDepth.setVideoMode(*it);
                VideoMode vm = vsDepth.getVideoMode();
                cout << formatIntrinsic( "Depth", vsDepth, vm ) << endl;
            }

            cout << "Extrinsic Parameters:" << endl;
            cout << formatExtrinsic( SENSOR_DEPTH, vsDepth, otherSensors );
        }
        vsDepth.destroy();
    }

    if ( device.hasSensor( SENSOR_IR ) )
    {
        VideoStream vsIr;
        if (STATUS_OK != vsIr.create(device, SENSOR_IR))
        {
            cout << "Cannot create infrared stream on device: " << OpenNI::getExtendedError() << endl;
        }
        else
        {
            cout << "Intrinsic Parameters:" << endl;

            const SensorInfo *sInfo = device.getSensorInfo( SENSOR_IR );
            const Array<VideoMode>& allvmodes = sInfo->getSupportedVideoModes();
            vector<VideoMode> vmodes = trimVideoModes( allvmodes );

            for ( auto it = vmodes.begin(); it != vmodes.end(); it++ )
            {
                vsIr.setVideoMode( *it );
                VideoMode vm = vsIr.getVideoMode();
                cout << formatIntrinsic( "IR", vsIr, vm ) << endl;
            }

            cout << "Extrinsic Parameters:" << endl;
            cout << formatExtrinsic( SENSOR_IR, vsIr, otherSensors );
        }
        vsIr.destroy();
    }

    if ( device.hasSensor( SENSOR_COLOR ) )
    {
        VideoStream vsColor;
        if (STATUS_OK != vsColor.create(device, SENSOR_COLOR))
        {
            cout << "Cannot create color stream on device: " << OpenNI::getExtendedError() << endl;
            //return 1;
        }
        else
        {
            cout << "Intrinsic Parameters:" << endl;

            const SensorInfo *sInfo = device.getSensorInfo( SENSOR_COLOR );
            const Array<VideoMode>& allvmodes = sInfo->getSupportedVideoModes();
            vector<VideoMode> vmodes = trimVideoModes( allvmodes );

            for ( auto it = vmodes.begin(); it != vmodes.end(); it++ )
            {
                vsColor.setVideoMode( *it );
                VideoMode vm = vsColor.getVideoMode();
                cout << formatIntrinsic( "Color", vsColor, vm ) << endl;
            }

            cout << "Extrinsic Parameters:" << endl;
            cout << formatExtrinsic( SENSOR_COLOR, vsColor, otherSensors );
        }
        vsColor.destroy();
    }

    device.close();
    OpenNI::shutdown();

    return 0;
}