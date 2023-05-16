#include <stdio.h>
#include <math.h>
#include "OpenNI.h"
#include "Device.h"

#define ARRAY_SIZE 100000

openni::Device g_device;

openni::VideoStream g_depthStream;
openni::VideoStream g_colorStream;

openni::VideoFrameRef g_depthFrame;
openni::VideoFrameRef g_colorFrame;

openni::VideoMode* g_depthVideoModes;
openni::VideoMode* g_colorVideoModes;

int g_depthVideoModeSize;
int g_colorVideoModeSize;
int g_selectedDepthRes;
int g_preSelectedDepthRes;

bool g_bIsColorValid;

int g_depthWidth, g_depthHeight;
bool g_bDepthUnit1mm;

int g_colorWidth, g_colorHeight;
float g_xzFactor, g_yzFactor;

struct pointData *g_pointsData;
int g_pointsDataSize;
int g_colorTable[1276][3];
int g_colorTableSize;

void initParam()
{
    g_bIsColorValid = false;
    g_pointsDataSize = 0;

    g_depthVideoModeSize = 0;
    g_colorVideoModeSize = 0;
    g_selectedDepthRes = 0;
    g_preSelectedDepthRes = 0;
}

void updatePointsDataSize()
{
    if ( g_pointsDataSize != 0 )
    {
        delete[] g_pointsData;
    }
    g_pointsDataSize = g_depthWidth * g_depthHeight;
    g_pointsData = new struct pointData[g_pointsDataSize];
}

void getDepthSupportedVideoModes()
{
    const openni::SensorInfo* sinfo = g_device.getSensorInfo( openni::SENSOR_DEPTH );
    const openni::Array< openni::VideoMode >& modesDepth = sinfo->getSupportedVideoModes();

    g_depthVideoModeSize = modesDepth.getSize();

    if ( g_depthVideoModeSize > 0 )
    {
        g_depthVideoModes = new openni::VideoMode[g_depthVideoModeSize];
        for ( int i = 0; i < g_depthVideoModeSize; i++ )
        {
            g_depthVideoModes[i] = modesDepth[i];
        }
    }
}

void getColorSupportedVideoModes()
{
    const openni::SensorInfo* sinfo = g_device.getSensorInfo( openni::SENSOR_COLOR );
    const openni::Array< openni::VideoMode >& modesColor = sinfo->getSupportedVideoModes();

    g_colorVideoModeSize = modesColor.getSize();

    if ( g_colorVideoModeSize > 0 )
    {
        g_colorVideoModes = new openni::VideoMode[g_colorVideoModeSize];
        for ( int i = 0; i < g_colorVideoModeSize; i++ )
        {
            g_colorVideoModes[i] = modesColor[i];
        }
    }
}

bool openDevice()
{
    openni::Status rc = openni::STATUS_OK;

    if ( openni::OpenNI::initialize() != openni::STATUS_OK )
    {
        printf( "OPENNI initialization:%s\n", openni::OpenNI::getExtendedError() );
        return false;
    }

    rc = g_device.open( openni::ANY_DEVICE );
    if ( rc != openni::STATUS_OK )
    {
        printf( "Device open failed:%s\n", openni::OpenNI::getExtendedError() );
        openni::OpenNI::shutdown();
        return false;
    }
    return true;
}

void closeDevice()
{
    g_depthStream.destroy();
    if ( g_bIsColorValid )
    {
        g_colorStream.destroy();
    }
    g_device.close();
    openni::OpenNI::shutdown();

    delete[] g_depthVideoModes;
    delete[] g_colorVideoModes;
    delete[] g_pointsData;
}

bool streamCreate()
{
    openni::Status rc;

    // Create depth stream
    rc = g_depthStream.create( g_device, openni::SENSOR_DEPTH );
    if ( rc != openni::STATUS_OK )
    {
        printf( "Couldn't create depth stream:%s\n", openni::OpenNI::getExtendedError() );
        return false;
    }

    // OpenNI World coordinate system is a 'left-handed system'
    // but OpenGL is a 'right-handed system'
    // so here we turn on mirroring to correct our view
    // Refs: http://docs.ros.org/en/kinetic/api/libfreenect/html/classopenni_1_1CoordinateConverter.html
    g_depthStream.setMirroringEnabled( true );

    // Create color stream
    rc = g_colorStream.create( g_device, openni::SENSOR_COLOR );
    if ( rc != openni::STATUS_OK )
    {
        printf( "Couldn't create color stream:%s\n", openni::OpenNI::getExtendedError() );
    }
    else
    {
        // OpenNI World coordinate system is a 'left-handed system'
        // but OpenGL is a 'right-handed system'
        // so here we turn on mirroring to correct our view
        // Refs: http://docs.ros.org/en/kinetic/api/libfreenect/html/classopenni_1_1CoordinateConverter.html
        g_colorStream.setMirroringEnabled( true );
    }

    // Check and enable Depth-To-Color image registration
    if ( g_colorStream.isValid() )
    {
        if ( g_device.isImageRegistrationModeSupported( openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR ) )
        {
            if ( openni::STATUS_OK == g_device.setImageRegistrationMode( openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR ) )
            {
                g_bIsColorValid = true;
            }
            else
            {
                printf( "ERROR: Failed to set imageRegistration mode\n\n" );
            }
        }
        else
        {
            printf( "ERROR: ImageRegistration mode is not supported\n\n" );
        }
    }

    // Get video modes of depth/color stream supported
    getDepthSupportedVideoModes();
    if ( g_bIsColorValid )
    {
        getColorSupportedVideoModes();
    }
    return true;
}

bool streamStart()
{
    if ( openni::STATUS_OK != g_depthStream.start() )
    {
        printf( "Couldn't start depth stream:%s\n", openni::OpenNI::getExtendedError() );
        return false;
    }
    if ( g_bIsColorValid )
    {
        if ( openni::STATUS_OK != g_colorStream.start() )
        {
            printf( "Couldn't start color stream:%s\n", openni::OpenNI::getExtendedError() );
            return false;
        }
    }
    return true;
}

void streamStop()
{
    g_depthStream.stop();
    if ( g_bIsColorValid )
    {
        g_colorStream.stop();
    }
    return;
}

bool setDepthVideoMode( int index )
{
    if ( g_depthVideoModeSize > 0 && index < g_depthVideoModeSize )
    {
        openni::Status rc = g_depthStream.setVideoMode( g_depthVideoModes[index] );
        if ( rc != openni::STATUS_OK )
        {
            printf( "ERROR: Failed to set video mode of depth stream.\n" );
            return false;
        }
        g_depthWidth = g_depthVideoModes[g_selectedDepthRes].getResolutionX();
        g_depthHeight = g_depthVideoModes[g_selectedDepthRes].getResolutionY();
        if ( g_depthVideoModes[index].getPixelFormat() == openni::PIXEL_FORMAT_DEPTH_1_MM )
        {
            g_bDepthUnit1mm = true;
        }
        else
        {
            g_bDepthUnit1mm = false;
        }
        return true;
    }
    return false;
}

void setColorVideoMode()
{
    if ( !g_bIsColorValid )
    {
        return;
    }

    int index = -1;
    for ( int i = 0; i < g_colorVideoModeSize; i++ )
    {
        if ( g_colorVideoModes[i].getResolutionX() == g_depthVideoModes[g_selectedDepthRes].getResolutionX()
                && g_colorVideoModes[i].getResolutionY() == g_depthVideoModes[g_selectedDepthRes].getResolutionY()
                && g_colorVideoModes[i].getFps() == g_depthVideoModes[g_selectedDepthRes].getFps()
                && g_colorVideoModes[i].getPixelFormat() == openni::PIXEL_FORMAT_RGB888 )
        {
            index = i;
        }
    }

    if ( index < 0 )
    {
        for ( int i = 0; i < g_colorVideoModeSize; i++ )
        {
            if ( g_colorVideoModes[i].getResolutionX() == g_depthVideoModes[g_selectedDepthRes].getResolutionX()
                    && g_colorVideoModes[i].getResolutionY() == g_depthVideoModes[g_selectedDepthRes].getResolutionY()
                    && g_colorVideoModes[i].getPixelFormat() == openni::PIXEL_FORMAT_RGB888 )
            {
                printf( "Warning: Depth FPS and Color FPS are different.\n" );
                index = i;
            }
        }

        if ( index < 0 )
        {
            for ( int i = 0; i < g_colorVideoModeSize; i++ )
            {
                if ( g_colorVideoModes[i].getResolutionX() == g_depthVideoModes[g_selectedDepthRes].getResolutionX()
                        && g_colorVideoModes[i].getPixelFormat() == openni::PIXEL_FORMAT_RGB888 )
                {
                    printf( "Warning: Depth resolution and Color resolution are different.\n" );
                    index = i;
                }
            }

            if ( index < 0 )
            {
                printf( "ERROR: Failed to search the video mode for color stream setting.\n" );
                return;
            }
        }
    }

    openni::Status rc = g_colorStream.setVideoMode( g_colorVideoModes[index] );
    if ( rc != openni::STATUS_OK )
    {
        printf( "ERROR: Failed to set video mode of color stream.\n" );
    }

    g_colorWidth = g_colorVideoModes[index].getResolutionX();
    g_colorHeight = g_colorVideoModes[index].getResolutionY();

    float colorHorizontalFov, colorVerticalFov;
    g_colorStream.getProperty( ONI_STREAM_PROPERTY_HORIZONTAL_FOV, &colorHorizontalFov );
    g_colorStream.getProperty( ONI_STREAM_PROPERTY_VERTICAL_FOV, &colorVerticalFov );

    g_xzFactor = tan( colorHorizontalFov / 2.0 ) * 2.0;
    g_yzFactor = tan( colorVerticalFov / 2.0 ) * 2.0;
}

bool readFrame()
{
    if ( g_depthStream.isValid() )
    {
        if ( openni::STATUS_OK != g_depthStream.readFrame( &g_depthFrame ) )
        {
            return false;
        }
    }
    if ( g_bIsColorValid && g_colorStream.isValid() )
    {
        if ( openni::STATUS_OK != g_colorStream.readFrame( &g_colorFrame ) )
        {
            return false;
        }
    }
    return true;
}

void colorMapInit()
{
    int i = 0;
    g_colorTableSize = 1276;
    for ( int j = 0; j <= 255; j++ )
    {
        g_colorTable[i][0] = 255;
        g_colorTable[i][1] = j;
        g_colorTable[i][2] = 0;
        i++;
    }
    for ( int j = 254; j >= 0; j-- )
    {
        g_colorTable[i][0] = j;
        g_colorTable[i][1] = 255;
        g_colorTable[i][2] = 0;
        i++;
    }
    for ( int j = 1; j <= 255; j++ )
    {
        g_colorTable[i][0] = 0;
        g_colorTable[i][1] = 255;
        g_colorTable[i][2] = j;
        i++;
    }
    for ( int j = 254; j >= 0; j-- )
    {
        g_colorTable[i][0] = 0;
        g_colorTable[i][1] = j;
        g_colorTable[i][2] = 255;
        i++;
    }
    for ( int j = 1; j <= 255; j++ )
    {
        g_colorTable[i][0] = j;
        g_colorTable[i][1] = 0;
        g_colorTable[i][2] = 255;
        i++;
    }
}

// When enable Depth-To-Color image registration, use RGB fov for coordinate transformation.
void convertDepthToWorldCoordinates( float depthX, float depthY, float depthZ, float* pWorldX, float* pWorldY, float* pWorldZ )
{
    float normalizedX = depthX / g_colorWidth - .5f;
    float normalizedY = .5f - depthY / g_colorHeight;
    *pWorldX = normalizedX * depthZ * g_xzFactor;
    *pWorldY = normalizedY * depthZ * g_yzFactor;
    *pWorldZ = depthZ;
}

void niComputeCloud()
{
    const openni::DepthPixel* pDepth = ( const openni::DepthPixel* )g_depthFrame.getData();
    const openni::RGB888Pixel* pColor;
    if ( g_bIsColorValid )
    {
        pColor = ( const openni::RGB888Pixel* )g_colorFrame.getData();
    }

    float fX, fY, fZ;
    float histogram[ARRAY_SIZE] = { 0.0f };
    int depthMax = 0;
    for ( int y = 0, i = 0; y < g_depthHeight; y++ )
    {
        for ( int x = 0; x < g_depthWidth; x++ )
        {
            fX = 0.0;
            fY = 0.0;
            fZ = 0.0;
            g_pointsData[i].depth = pDepth[i];
            if ( pDepth[i] != 0 )
            {
                if ( g_bIsColorValid )
                {
                    convertDepthToWorldCoordinates( static_cast<float>( x ), static_cast<float>( y ), static_cast<float>( pDepth[i] ), &fX, &fY, &fZ );
                }
                else
                {
                    openni::CoordinateConverter::convertDepthToWorld( g_depthStream, x, y, pDepth[i], &fX, &fY, &fZ );
                }

                // Set the unit to 1 mm for drawing the point cloud
                if ( !g_bDepthUnit1mm )
                {
                    fX /= 10.0f;
                    fY /= 10.0f;
                    fZ /= 10.0f;
                }

                g_pointsData[i].worldX = fX;
                g_pointsData[i].worldY = fY;
                g_pointsData[i].worldZ = fZ;
                if ( g_bIsColorValid )
                {
                    g_pointsData[i].r = pColor[i].r / 255.0;
                    g_pointsData[i].g = pColor[i].g / 255.0;
                    g_pointsData[i].b = pColor[i].b / 255.0;
                    g_pointsData[i].ucharR = pColor[i].r;
                    g_pointsData[i].ucharG = pColor[i].g;
                    g_pointsData[i].ucharB = pColor[i].b;
                }
                if ( pDepth[i] < ARRAY_SIZE )
                {
                    histogram[pDepth[i]]++;
                    if ( pDepth[i] > depthMax )
                    {
                        depthMax = pDepth[i];
                    }
                }
            }
            i++;
        }
    }

    for ( int i = 1; i <= depthMax; i++ )
    {
        histogram[i] += histogram[i - 1];
    }
    for ( int i = 0; i < g_pointsDataSize; i++ )
    {
        if ( g_pointsData[i].depth != 0 )
        {
            if ( g_pointsData[i].depth <= depthMax )
            {
                int index = histogram[g_pointsData[i].depth] / histogram[depthMax] * ( g_colorTableSize - 1 );
                g_pointsData[i].colorMapR = g_colorTable[index][0] / 255.0;
                g_pointsData[i].colorMapG = g_colorTable[index][1] / 255.0;
                g_pointsData[i].colorMapB = g_colorTable[index][2] / 255.0;
            }
            else
            {
                g_pointsData[i].colorMapR = 0.0f;
                g_pointsData[i].colorMapG = 0.0f;
                g_pointsData[i].colorMapB = 0.0f;
            }
        }
    }
    return;
}
